#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include "common/common.h"

#define SERVER_TOKEN 1
#define CLIENT_TOKEN 2

void eventfd_notify(int descriptor, uint64_t value) {
	if (write(descriptor, &value, 8) == -1) {
		throw("Error writing to eventfd");
	}
}

void eventfd_wait(int descriptor, uint64_t wanted) {
	uint64_t stored;

	while (true) {
		// A read from an eventfd returns the 8-byte integer
		// stored in the eventfd object *and* resets the value
		// to zero. That is, unless the EFD_SEMAPHORE flag was
		// passed at the start. In that case, the returned value
		// is *always* 1 and the stored value is decremented by
		// 1 (not reset to zero).
		if (read(descriptor, &stored, 8) == -1) {
			throw("Error reading from eventfd");
		}

		if (stored == wanted) {
			return;
		}

		// If this was not the value we were looking for
		// (e.g. if the server read the signal it set)
		// then write it back.
		if (write(descriptor, &stored, 8) == -1) {
			throw("Error writing back to eventfd");
		}
	}
}

void client_communicate(int descriptor, struct Arguments* args) {
	for (; args->count > 0; --args->count) {
		eventfd_notify(descriptor, SERVER_TOKEN);
		eventfd_wait(descriptor, CLIENT_TOKEN);
	}
}


void server_communicate(int descriptor, struct Arguments* args) {
	struct Benchmarks bench;
	int message;

	setup_benchmarks(&bench);

	for (message = 0; message < args->count; ++message) {
		bench.single_start = now();

		eventfd_wait(descriptor, SERVER_TOKEN);
		eventfd_notify(descriptor, CLIENT_TOKEN);

		benchmark(&bench);
	}

	// The message size is always one (it's just a signal)
	args->size = 1;
	evaluate(&bench, args);
}

void communicate(int descriptor, struct Arguments* args) {
	// File descriptors can only be shared bewteen related processes,
	// therefore we will need to fork this process
	pid_t pid;

	// Fork a child process
	if ((pid = fork()) == -1) {
		throw("Error forking process");
	}

	// fork() returns 0 for the child process
	if (pid == (pid_t)0) {
		client_communicate(descriptor, args);
		close(descriptor);
	} else {
		server_communicate(descriptor, args);
	}
}

int main(int argc, char* argv[]) {
	// An eventfd object is simply a file in the file-system, that can
	// be used as a wait/notify-mechanism similar to a semaphore. It
	// is associated with a standard file descriptor and thus the
	// usual read() and write() functions can be used, albeit their
	// behaviour is different than for standard files.
	// Stored in the eventfd itself is a simple 64-bit/8-Byte integer.
	int descriptor;

	struct Arguments args;
	parse_arguments(&args, argc, argv);

	// Create a new eventfd object and get the corresponding
	// file descriptor. The first argument is the initial value,
	// which we just set to 0 here and the second argument are
	// any flags (a bitwise-OR combination thereof), such as:
	// EFD_NONBLOCK: Causes read() and write() to return immdediately
	//               where the calls would normally block. In that case,
	//               the return code is EAGAIN.
	// EFD_SEMAPHORE: Causes reads to always return a value of 1 and
	//                decrement the value stored in the eventfd by 1.
	descriptor = eventfd(0, 0);

	communicate(descriptor, &args);

	return EXIT_SUCCESS;
}
