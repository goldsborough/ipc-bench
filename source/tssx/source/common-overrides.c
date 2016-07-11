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
	printf("fork\n");
	if (bridge_is_initialized(&bridge)) {
		// Increments all reference counts
		bridge_add_user(&bridge);
	}
	return real_fork();
}

/******************** INTERFACE ********************/

ssize_t connection_write(int key,
										 const  void* source,
										 size_t requested_bytes,
										 int which_buffer) {
	Session* session;

	if (key < TSSX_KEY_OFFSET) {
		return real_write(key, source, requested_bytes);
	} else {
		session = bridge_lookup(&bridge, key);
		assert(session_is_valid(session));
		// Check if the session is actually a TSSX session or a standard domain
		// socket that we just had to put in here on client side
		if (session->connection == NULL) {
			return real_write(session->socket, source, requested_bytes);
		} else {
			return buffer_write(
					get_buffer(session->connection, which_buffer),
					source,
					requested_bytes
			);

			printf("writing: '%.*s' (%i)\n", (int)requested_bytes, (char*)source, (int)requested_bytes);

			// clang-format off
			ssize_t res = buffer_write(
					get_buffer(session->connection, which_buffer),
					source,
					requested_bytes
			);
			// clang-format on

			printf("tssx wrote: %li\n", res);

			res = real_write(session->socket, source, requested_bytes);

			printf("domain wrote: %li\n", res);

			return res;
		}
	}
}

ssize_t connection_read(int key,
										void* destination,
										size_t requested_bytes,
										int which_buffer) {
	Session* session;

	if (key < TSSX_KEY_OFFSET) {
		return real_read(key, destination, requested_bytes);
	} else {
		session = bridge_lookup(&bridge, key);
		assert(session_is_valid(session));
		if (session->connection == NULL) {
			return real_read(session->socket, destination, requested_bytes);
		} else {
			return buffer_read(
					get_buffer(session->connection, which_buffer),
					destination,
					requested_bytes
			);

			printf("reading ...\n");

			ssize_t res = real_read(session->socket, destination, requested_bytes);

			printf("domain: %i\n", (int)res);
			if(res > 0)
				printf("content: '%.*s'\n", (int)res, (char*)destination);
			else
				printf("read nothing\n");

			// clang-format off
			res = buffer_read(
					get_buffer(session->connection, which_buffer),
					destination,
					requested_bytes
			);
			// clang-format on

			printf("tssx:\n");
			if(res > 0)
				printf("content: '%.*s'\n", (int)res, (char*)destination);
			else
				printf("read nothing\n");

			return res;
		}
	}
}

int socket_is_stream_and_domain(int domain, int type) {
	/*
	* The only point of this function is that we are allowed to include *
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
	if (fd < TSSX_KEY_OFFSET) {
		return real_fcntl_set_flags(fd, command, flags);
	} else {
		Session* session = bridge_lookup(&bridge, fd);

		// Seems the user passed an invalid socket FD
		if (!session_is_valid(session)) {
			errno = EINVAL;
			return ERROR;
		}

      // We always forward "set operations" to the underlying socket
      // This way we can just read them back, when the tssx connection is created
      return real_fcntl_set_flags(session->socket, command, flags);

      // If we have already created a valid tssx connection, we set the options there
		if (session->connection == NULL) {
         bool non_blocking = flags & O_NONBLOCK;
         set_non_blocking(session->connection, non_blocking);
		}
	}

	return SUCCESS;
}

int fcntl_get(int fd, int command) {
	if (fd < TSSX_KEY_OFFSET) {
		return real_fcntl_get_flags(fd, command);
	} else {
		Session* session = bridge_lookup(&bridge, fd);

		// Seems the user passed an invalid socket FD
		if (!session_is_valid(session)) {
			errno = EINVAL;
			return ERROR;
		}

      // All get operations can simply be forwarded to the
      // underlying socket, because all set operations were
      // forwarded as well.
      return real_fcntl_get_flags(session->socket, command);
	}
}
