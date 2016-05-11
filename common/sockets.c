#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#include "common/sockets.h"
#include "common/utility.h"

void adjust_socket_individual_buffer_size(int socket_descriptor, int which) {
	int buffer_size = BUFFER_SIZE;

	// set/getsockopt is the one-stop-shop for all socket options.
	// With it, you can get/set a variety of options for a given socket.
	// Arguments:
	// 1. The socket file-descriptor.
	// 2. The level of the socket, should be SOL_SOCKET.
	// 3. The option you want to get/set.
	// 4. The address of a value you want to get/set into.
	// 5. The memory-size of said value.
	// clang-format off

	// Set the sockets send-buffer-size (SNDBUF)
	int return_code = setsockopt(
		socket_descriptor,
		SOL_SOCKET,
		which,
		&buffer_size,
		sizeof buffer_size
	);
	// clang-format on

	if (return_code == -1) {
		throw("Error setting socket buffer size");
	}
}

void adjust_socket_buffer_size(int socket_descriptor) {
	// clang-format off
	adjust_socket_individual_buffer_size(
		socket_descriptor,
		SO_SNDBUF
	 );

	adjust_socket_individual_buffer_size(
		socket_descriptor,
		SO_RCVBUF
	 );
	// clang-format on
}

void adjust_socket_blocking_timeout(int socket_descriptor,
																		int seconds,
																		int microseconds) {
	struct timeval timeout = {seconds, microseconds};

	// clang-format off
	int return_code = setsockopt(
		socket_descriptor,
		SOL_SOCKET,
		SO_RCVTIMEO,
		&timeout,
		sizeof timeout
	);
	// clang-format on

	if (return_code == -1) {
		throw("Error setting blocking timeout");
	}
}


int set_socket_flag(int socket_descriptor, int flag) {
	int old_flags;

	// Get the old flags, because we must bitwise-OR our flag to add it
	// fnctl takes as arguments:
	// 1. The file descriptor to modify
	// 2. The command (e.g. F_GETFL, F_SETFL, ...)
	// 3. Arguments to that command (variadic)
	// For F_GETFL, the arguments are ignored (that's why we pass 0)
	if ((old_flags = fcntl(socket_descriptor, F_GETFL, 0)) == -1) {
		return -1;
	}

	if (fcntl(socket_descriptor, F_SETFL, old_flags | flag)) {
		return -1;
	}

	return 0;
}

int receive(int connection, void* buffer, int size, int busy_waiting) {
	if (busy_waiting) {
		while (recv(connection, buffer, size, 0) < size) {
			if (errno != EAGAIN) return -1;
		}
	} else if (recv(connection, buffer, size, 0) < size) {
		return -1;
	}

	return 0;
}
