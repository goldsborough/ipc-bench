#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#include "common/sockets.h"
#include "common/utility.h"

typedef struct timeval timeval;

int get_socket_buffer_size(int socket_fd, int which) {
	int return_code;
	int buffer_size;
	socklen_t value_size = sizeof buffer_size;

	// clang-format off
	return_code = getsockopt(
		socket_fd,
		SOL_SOCKET,
		(which == SEND) ? SO_SNDBUF : SO_RCVBUF,
		&buffer_size,
		&value_size
	);
	// clang-format on

	if (return_code == -1) {
		throw("Error getting buffer size");
	}

	return buffer_size;
}

timeval get_socket_timeout(int socket_fd, int which) {
	int return_code;
	timeval timeout;
	socklen_t value_size = sizeof timeout;

	// clang-format off
	return_code = getsockopt(
		socket_fd,
		SOL_SOCKET,
		(which == SEND) ? SO_SNDTIMEO : SO_RCVTIMEO,
		&timeout,
		&value_size
	);
	// clang-format on

	if (return_code == -1) {
	  throw("Error getting socket timeout");
	}

	return timeout;
}

double get_socket_timeout_seconds(int socket_fd, int which) {
	timeval timeout = get_socket_timeout(socket_fd, which);
	return timeout.tv_sec + (timeout.tv_usec / 1e6);
}

void set_socket_buffer_size(int socket_fd, int which) {
	int buffer_size = BUFFER_SIZE;

	// set/getsockopt is the one-stop-shop for all socket options.
	// With it, you can get/set a variety of options for a given socket.
	// Arguments:
	// 1. The socket file-fd.
	// 2. The level of the socket, should be SOL_SOCKET.
	// 3. The option you want to get/set.
	// 4. The address of a value you want to get/set into.
	// 5. The memory-size of said value.
	// clang-format off

	// Set the sockets send-buffer-size (SNDBUF)
	int return_code = setsockopt(
		socket_fd,
		SOL_SOCKET,
		(which == SEND) ? SO_SNDBUF : SO_RCVBUF,
		&buffer_size,
		sizeof buffer_size
	);
	// clang-format on

	if (return_code == -1) {
		throw("Error setting socket buffer size");
	}
}

void set_socket_both_buffer_sizes(int socket_fd) {
	// clang-format off
	set_socket_buffer_size(
		socket_fd,
		SEND
	 );

	set_socket_buffer_size(
		socket_fd,
		RECEIVE
	 );
	// clang-format on
}

void set_socket_timeout(int socket_fd, timeval* timeout, int which) {
	// clang-format off
	int return_code = setsockopt(
		socket_fd,
		SOL_SOCKET,
		(which == SEND) ? SO_SNDTIMEO : SO_RCVTIMEO,
		timeout,
		sizeof *timeout
	);
	// clang-format on

	if (return_code == -1) {
		throw("Error setting blocking timeout");
	}
}

void set_socket_both_timeouts(int socket_fd, int seconds, int microseconds) {
	struct timeval timeout = {seconds, microseconds};

	set_socket_timeout(socket_fd, &timeout, SEND);
	set_socket_timeout(socket_fd, &timeout, RECEIVE);
}


int set_io_flag(int socket_fd, int flag) {
	int old_flags;

	// Get the old flags, because we must bitwise-OR our flag to add it
	// fnctl takes as arguments:
	// 1. The file fd to modify
	// 2. The command (e.g. F_GETFL, F_SETFL, ...)
	// 3. Arguments to that command (variadic)
	// For F_GETFL, the arguments are ignored (that's why we pass 0)
	if ((old_flags = fcntl(socket_fd, F_GETFL, 0)) == -1) {
		return -1;
	}

	if (fcntl(socket_fd, F_SETFL, old_flags | flag)) {
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
