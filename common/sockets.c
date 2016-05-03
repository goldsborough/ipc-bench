#include "common/sockets.h"
#include "common/utility.h"

void adjust_socket_buffer_size(int socket_descriptor, int message_size) {
	int return_code;

	// The minimum allowed value is 1024
	// http://man7.org/linux/man-pages/man7/socket.7.html
	if (message_size < 1024) {
		message_size = 1024;
	}

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
	return_code = setsockopt(
		socket_descriptor,
		SOL_SOCKET,
		SO_SNDBUF,
		&message_size,
		sizeof message_size
	);

	if (return_code == -1) {
		throw("Error setting socket send buffer size");
	}

	// Set the socket's receive-buffer-size (RCVBUF)
	return_code = setsockopt(
		socket_descriptor,
		SOL_SOCKET,
		SO_RCVBUF,
		&message_size,
		sizeof message_size
  );
	// clang-format on

	if (return_code == -1) {
		throw("Error setting socket receive-buffer-size");
	}
}
