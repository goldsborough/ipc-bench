#define _GNU_SOURCE

#include <assert.h>
#include <dlfcn.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "tssx/bridge.h"
#include "tssx/buffer.h"
#include "tssx/overrides.h"
#include "tssx/session.h"

/******************** REAL FUNCTIONS ********************/

// Socket API
// RTDL_NEXT = look in the symbol table of the *next* object file after this one
int real_socket(int domain, int type, int protocol) {
	return ((real_socket_t)dlsym(RTLD_NEXT, "socket"))(domain, type, protocol);
}

ssize_t real_write(int fd, const void* data, size_t size) {
	return ((real_write_t)dlsym(RTLD_NEXT, "write"))(fd, data, size);
}

ssize_t real_read(int fd, void* data, size_t size) {
	return ((real_read_t)dlsym(RTLD_NEXT, "read"))(fd, data, size);
}

int real_accept(int fd, sockaddr* address, socklen_t* length) {
	return ((real_accept_t)dlsym(RTLD_NEXT, "accept"))(fd, address, length);
}

int real_connect(int fd, const sockaddr* address, socklen_t length) {
	return ((real_connect_t)dlsym(RTLD_NEXT, "connect"))(fd, address, length);
}

int real_getsockopt(int fd,
										int level,
										int option_name,
										void* restrict option_value,
										socklen_t* restrict option_len) {
	// Some nice lisp here
	// clang-format off
	return ((real_getsockopt_t)dlsym(RTLD_NEXT, "getsockopt"))
      (fd, level, option_name, option_value, option_len);
	// clang-format on
}

int real_setsockopt(int fd,
										int level,
										int option_name,
										const void* option_value,
										socklen_t option_len) {
	// Some nice lisp here
	// clang-format off
	return ((real_setsockopt_t)dlsym(RTLD_NEXT, "setsockopt"))
      (fd, level, option_name, option_value, option_len);
	// clang-format on
}

int real_close(int fd) {
	return ((real_close_t)dlsym(RTLD_NEXT, "close"))(fd);
}

// Other
pid_t real_fork(void) {
	return ((real_fork_t)dlsym(RTLD_NEXT, "fork"))();
}

/******************** COMMON OVERRIDES ********************/

int getsockopt(int key,
							 int level,
							 int option_name,
							 void* restrict option_value,
							 socklen_t* restrict option_len) {
	// clang-format off
	return real_getsockopt(
			bridge_deduce_file_descriptor(&bridge, key),
      level,
      option_name,
      option_value,
      option_len
  );
  // clang-fomat pm
}

int setsockopt(int key,
							 int level,
							 int option_name,
							 const void* option_value,
							 socklen_t option_len) {
  // clang-format off
  return real_setsockopt(
     bridge_deduce_file_descriptor(&bridge, key),
     level,
     option_name,
     option_value,
     option_len
  );
  // clang-fomat pm
}

pid_t fork() {
	if (bridge_is_initialized(&bridge)) {
    // Increments all reference counts
  	bridge_add_user(&bridge);
  }
	return real_fork();
}

int close(int key) {
	if (key >= TSSX_KEY_OFFSET) {
		bridge_free(&bridge, key);
	}

	return real_close(key);
}

/******************** INTERFACE ********************/

int connection_write(int key,
										 void* source,
										 int requested_bytes,
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
			// clang-format off
			return buffer_write(
        get_buffer(session->connection, which_buffer),
        source,
        requested_bytes
      );
			// clang-format on
		}
	}
}

int connection_read(int key,
										void* destination,
										int requested_bytes,
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
			// clang-format off
			return buffer_read(
        get_buffer(session->connection, which_buffer),
        destination,
        requested_bytes
      );
			// clang-format on
		}
	}
}

Buffer* get_buffer(Connection* connection, int which_buffer) {
	return which_buffer ? connection->client_buffer : connection->server_buffer;
}

int socket_is_stream_and_domain(int domain, int type) {
	/*
	* The only point of this function is that we are allowed to include *
	* <sys/socket.h> here and access the AF_LOCAL symbolic name, while we're not
	* allowed in the server/client-overrides file. We could just hardcode the
	* constant for AF_LOCAL in those files, but that wouldn't be sustainable.
	*
	* Note that we'll only want to use tssx for stream (TCP-like) oriented
	* sockets, not datagram (UDP-like) sockets
	*/
	return domain == AF_LOCAL && type == SOCK_STREAM;
}
