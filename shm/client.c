#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <unistd.h>

#include "common/common.h"

void communicate(char* shared_memory,
								 struct sigaction* signal_action,
								 struct Arguments* args) {
	// Buffer into which to read data
	void* buffer = malloc(args->size);

	// First signal to set things going
	client_signal();

	for (; args->count > 0; --args->count) {
		wait_for_signal(signal_action);

		// Read
		memmove(buffer, shared_memory, args->size);

		// Write back
		memset(shared_memory, '*', args->size);

		client_signal();
	}

	free(buffer);
}

int main(int argc, char* argv[]) {
	// The identifier for the shared memory segment
	int segment_id;

	// For sending signals
	struct sigaction signal_action;

	// The *actual* shared memory, that this and other
	// processes can read and write to as if it were
	// any other plain old memory
	char* shared_memory;

	// Fetch command-line arguments
	struct Arguments args;

	parse_arguments(&args, argc, argv);
	setup_client_signals(&signal_action);

	// Wait until we can fetch the memory
	wait_for_signal(&signal_action);

	/*
		The call that actually allocates the shared memory segment.
		Arguments:
			1. The shared memory key. This must be unique across the OS.
			2. The number of bytes to allocate. This will be rounded up to the OS'
				 pages size for alignment purposes.
			3. The creation flags and permission bits, because we assume the
				 segment already exists and need no longer be created, we don't need
				 any creation flags such as IPC_CREAT here. The permission flags will
				 do. 0666 means read + write permission for the user, group and world.
		The call will return the segment ID if the key was valid,
		else the call fails.
	*/
	segment_id = shmget(6969, args.size, 0666);

	if (segment_id < 0) {
		throw("Could not get segment");
	}

	/*
	Once the shared memory segment has been created, it must be
	attached to the address space of each process that wishes to
	use it. For this, we pass:
		1. The segment ID returned by shmget
		2. A pointer at which to attach the shared memory segment. This
			 address must be page-aligned. If you simply pass NULL, the OS
			 will find a suitable region to attach the segment.
		3. Flags, such as:
			 - SHM_RND: round the second argument (the address at which to
				 attach) down to a multiple of the page size. If you don't
				 pass this flag but specify a non-null address as second argument
				 you must ensure page-alignment yourself.
			 - SHM_RDONLY: attach for reading only (independent of access bits)
	shmat will return a pointer to the address space at which it attached the
	shared memory. Children processes created with fork() inherit this segment.
*/
	shared_memory = (char*)shmat(segment_id, NULL, 0);

	if (shared_memory < (char*)0) {
		throw("Could not attach segment");
	}

	communicate(shared_memory, &signal_action, &args);

	// Detach the shared memory from this process' address space.
	// If this is the last process using this shared memory, it is removed.
	shmdt(shared_memory);

	return EXIT_SUCCESS;
}
