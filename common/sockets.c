#include <stdio.h>

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
