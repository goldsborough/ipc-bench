#define _GNU_SOURCE

#include <assert.h>
#include <dlfcn.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "tssx/bridge.h"
#include "tssx/connection.h"
#include "tssx/session.h"
#include "tssx/socket-overrides.h"

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

int real_close(int fd) {
	return ((real_close_t)dlsym(RTLD_NEXT, "close"))(fd);
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

int real_getsockname(int sockfd,
							struct sockaddr *addr,
							socklen_t *addrlen) {
	// Some nice lisp here
	// clang-format off
	return ((real_getsockname_t)dlsym(RTLD_NEXT, "getsockname"))
			(sockfd, addr, addrlen);
	// clang-format on
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

int getsockname(int sockfd,
					struct sockaddr *addr,
					socklen_t *addrlen) {
	// clang-format off
	return real_getsockname(
			bridge_deduce_file_descriptor(&bridge, sockfd),
			addr,
			addrlen
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

int close(int key) {
	if (key >= TSSX_KEY_OFFSET) {
		bridge_free(&bridge, key);
	}

	return real_close(key);
}
