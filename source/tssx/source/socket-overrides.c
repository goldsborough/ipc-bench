#define _GNU_SOURCE

#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "common/utility.h"
#include "tssx/bridge.h"
#include "tssx/connection.h"
#include "tssx/session.h"
#include "tssx/socket-overrides.h"

/******************** REAL FUNCTIONS ********************/

// RTDL_NEXT = look in the symbol table of the *next* object file after this one
ssize_t real_write(int fd, const void* data, size_t size) {
	return ((real_write_t)dlsym(RTLD_NEXT, "write"))(fd, data, size);
}

ssize_t real_read(int fd, void* data, size_t size) {
	return ((real_read_t)dlsym(RTLD_NEXT, "read"))(fd, data, size);
}

ssize_t real_send(int fd, const void* buffer, size_t length, int flags) {
	return ((real_send_t)dlsym(RTLD_NEXT, "send"))(fd, buffer, length, flags);
}

ssize_t real_recv(int fd, void* buffer, size_t length, int flags) {
	return ((real_recv_t)dlsym(RTLD_NEXT, "recv"))(fd, buffer, length, flags);
}

ssize_t real_sendmsg(int fd, const struct msghdr* message, int flags) {
	return ((real_sendmsg_t)dlsym(RTLD_NEXT, "sendmsg"))(fd, message, flags);
}

ssize_t real_recvmsg(int fd, struct msghdr* message, int flags) {
	return ((real_recvmsg_t)dlsym(RTLD_NEXT, "recvmsg"))(fd, message, flags);
}

ssize_t real_sendto(int fd,
										const void* buffer,
										size_t length,
										int flags,
										const struct sockaddr* dest_addr,
										socklen_t dest_len) {
	// clang-format off
	return ((real_sendto_t)dlsym(RTLD_NEXT, "sendto"))
            (fd, buffer, length, flags, dest_addr, dest_len);
	// clang-format on
}

ssize_t real_recvfrom(int fd,
											void* restrict buffer,
											size_t length,
											int flags,
											struct sockaddr* restrict address,
											socklen_t* restrict address_len) {
	// clang-format off
	return ((real_recvfrom_t)dlsym(RTLD_NEXT, "recvfrom"))
            (fd, buffer, length, flags, address, address_len);
	// clang-format on
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

int real_getsockname(int fd, struct sockaddr* addr, socklen_t* addrlen) {
	// Some nice lisp here
	// clang-format off
	return ((real_getsockname_t)dlsym(RTLD_NEXT, "getsockname"))
			(fd, addr, addrlen);
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
			key,
      level,
      option_name,
      option_value,
      option_len
  );
	// clang-format on
}

int getsockname(int key, struct sockaddr* addr, socklen_t* addrlen) {
	return real_getsockname(key, addr, addrlen);
}

int setsockopt(int key,
							 int level,
							 int option_name,
							 const void* option_value,
							 socklen_t option_len) {
	// clang-format off
  return real_setsockopt(
     key,
     level,
     option_name,
     option_value,
     option_len
  );
  // clang-fomat pm
}

int close(int key) {
	bridge_free(&bridge, key);
	return real_close(key);
}

ssize_t send(int fd, const void* buffer, size_t length, int flags) {
// For now: We forward the call to write for a certain set of
// flags, which we chose to ignore. By putting them here explicitly,
// we make sure that we only ignore flags, which are not important.
// For production, we might wanna handle these flags
#ifdef __APPLE__
	if (flags == 0) {
#else
	if (flags == 0 || flags == MSG_NOSIGNAL) {
#endif
		return write(fd, buffer, length);
	} else {
    warn("Routing send to socket (unsupported flags)");
    return real_send(fd, buffer, length, flags);
  }
}

ssize_t recv(int fd, void *buffer, size_t length, int flags) {
#ifdef __APPLE__
	if (flags == 0) {
#else
	if (flags == 0 || flags == MSG_NOSIGNAL) {
#endif
		return read(fd, buffer, length);
	} else {
    warn("Routing recv to socket (unsupported flags)");
    return real_recv(fd, buffer, length, flags);
  }
}

ssize_t sendto(int fd,
							 const void *buffer,
							 size_t length,
							 int flags,
							 const struct sockaddr *dest_addr,
							 socklen_t addrlen) {
  // When the destination address is null, then this should be a stream socket
	if (dest_addr == NULL) {
    return send(fd, buffer, length, flags);
  } else {
    // Connection-Less sockets (UDP) sockets never use TSSX anyway
    return real_sendto(fd, buffer, length, flags, dest_addr, addrlen);
  }
}

ssize_t recvfrom(int fd,
								 void *buffer,
								 size_t length,
								 int flags,
								 struct sockaddr *src_addr,
								 socklen_t *addrlen) {
  // When the destination address is null, then this should be a stream socket
  if (src_addr == NULL) {
   return recv(fd, buffer, length, flags);
  } else {
   // Connection-Less sockets (UDP) sockets never use TSSX anyway
   return real_recvfrom(fd, buffer, length, flags, src_addr, addrlen);
  }
}

ssize_t sendmsg(int fd, const struct msghdr *msg, int flags) {
    // This one is hard to implemenet because the `msghdr` struct contains
    // an iovec pointer, which points to an array of iovec structs. Each such
    // struct is then a vector with a starting address and length. The sendmsg
    // call then fills these vectors one by one until the stream is empty or
    // all the vectors have been filled. I don't know how many people use this
    // function, but right now we just support a single buffer and else route
    // the call to the socket itself.
    if (msg->msg_iovlen == 1) {
      return sendto(fd, msg->msg_iov[0].iov_base, msg->msg_iov[0].iov_len, flags, (struct sockaddr*)msg->msg_name, msg->msg_namelen);
    } else {
      warn("Routing sendmsg to socket (too many buffers)");
      return real_sendmsg(fd, msg, flags);
    }
}

ssize_t recvmsg(int fd, struct msghdr *msg, int flags) {
  if (msg->msg_iovlen == 1) {
    return recvfrom(fd, msg->msg_iov[0].iov_base, msg->msg_iov[0].iov_len, flags, (struct sockaddr*)msg->msg_name, &msg->msg_namelen);
  } else {
    warn("Routing recvmsg to socket (too many buffers)");
    return real_recvmsg(fd, msg, flags);
  }
}
