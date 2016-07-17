#define _GNU_SOURCE

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/socket.h>

#include "tssx/bridge.h"
#include "tssx/buffer.h"
#include "tssx/common-overrides.h"

/******************** REAL FUNCTIONS ********************/

int real_fcntl_set_flags(int fd, int command, int flag) {
	return ((real_fcntl_t)dlsym(RTLD_NEXT, "fcntl"))(fd, command, flag);
}

int real_fcntl_get_flags(int fd, int command) {
	return ((real_fcntl_t)dlsym(RTLD_NEXT, "fcntl"))(fd, command);
}

pid_t real_fork(void) {
	return ((real_fork_t)dlsym(RTLD_NEXT, "fork"))();
}

/******************** COMMON OVERRIDES ********************/

int fcntl(int fd, int command, ...) {
	va_list argument;

	// Takes the argument pointer and the last positional argument
	// Makes the argument pointer point to the first optional argument
	va_start(argument, command);

	if (command == F_SETFL || command == F_SETFD) {
		return fcntl_set(fd, command, va_arg(argument, int));
	} else if (command == F_GETFL || command == F_GETFD) {
		return fcntl_get(fd, command);
	} else {
		// Sorry, don't know what to do for other commands :(
		// If necessary: handle all cases of arguments ...
		return ERROR;
	}
}

pid_t fork() {
	bridge_add_user(&bridge);
	return real_fork();
}

/******************** INTERFACE ********************/

ssize_t connection_write(int key,
												 const void* source,
												 size_t requested_bytes,
												 int which_buffer) {
	Session* session;

	session = bridge_lookup(&bridge, key);
	if (session_has_connection(session)) {
		// clang-format off
    return buffer_write(
        get_buffer(session->connection, which_buffer),
        source,
        requested_bytes
    );
		// clang-format on
	} else {
		return real_write(key, source, requested_bytes);
	}
}

ssize_t connection_read(int key,
												void* destination,
												size_t requested_bytes,
												int which_buffer) {
	Session* session;

	session = bridge_lookup(&bridge, key);
	if (session_has_connection(session)) {
		// clang-format off
    return buffer_read(
        get_buffer(session->connection, which_buffer),
        destination,
        requested_bytes
    );
		// clang-format on
	} else {
		return real_read(key, destination, requested_bytes);
	}
}

int socket_is_stream_and_domain(int domain, int type) {
	/*
	* The only point of this function is that we are allowed to include
	* <sys/socket.h> here and access the AF_LOCAL symbolic name, while we're not
	* allowed in the server/client-overrides file. We could just hardcode the
	* constant for AF_LOCAL in those files, but that wouldn't be sustainable.
	* Actually, we are allowed, it's just a mess to replicate the prototypes of
	* the overwritten functions exactly (especially since they're sometimes
	* annotated with OS X extensions ...)
	*
	* Note that we'll only want to use tssx for stream (TCP-like) oriented
	* sockets, not datagram (UDP-like) sockets
	*/
	return domain == AF_LOCAL && type == SOCK_STREAM;
}

/******************** HELPERS ********************/

Buffer* get_buffer(Connection* connection, int which_buffer) {
	return which_buffer ? connection->client_buffer : connection->server_buffer;
}

int fcntl_set(int fd, int command, int flags) {
	Session* session;
	int return_code;

	// Always set it on the socket (we can try to keep the flags in sync)
	if ((return_code = real_fcntl_set_flags(fd, command, flags)) == ERROR) {
		return ERROR;
	}

	session = bridge_lookup(&bridge, fd);
	if (session_has_connection(session)) {
		// Polymorphic call (implemented by server/client in their overrides)
		set_non_blocking(session->connection, flags & O_NONBLOCK);
	}

	return SUCCESS;
}

int fcntl_get(int fd, int command) {
	Session* session;
	int flags;

	flags = real_fcntl_get_flags(fd, command);

	// Theoretically the flags should be in sync
	// But we will get the non-blocking property, just in case
	session = bridge_lookup(&bridge, fd);
	if (session_has_connection(session)) {
		// First unset the flag, then check if we have it set
		flags &= ~O_NONBLOCK;
		if (is_non_blocking(session->connection)) {
			flags |= O_NONBLOCK;
		}
	}

	return flags;
}
