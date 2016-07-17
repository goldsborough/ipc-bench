#ifndef SOCKET_OVERRIDES_H
#define SOCKET_OVERRIDES_H

#include <sys/types.h>

/******************** DEFINITIONS ********************/

typedef struct sockaddr sockaddr;
typedef struct msghdr msghdr;

typedef unsigned int socklen_t;

typedef int (*real_socket_t)(int, int, int);

typedef int (*real_accept_t)(int, sockaddr*, socklen_t*);
typedef int (*real_connect_t)(int, const sockaddr*, socklen_t);

typedef ssize_t (*real_write_t)(int, const void*, size_t);
typedef ssize_t (*real_read_t)(int, void*, size_t);

typedef ssize_t (*real_send_t)(int, const void*, size_t, int);
typedef ssize_t (*real_recv_t)(int, void*, size_t, int);

typedef ssize_t (*real_sendmsg_t)(int, const struct msghdr*, int);
typedef ssize_t (*real_recvmsg_t)(int, struct msghdr*, int);

// clang-format off
typedef ssize_t (*real_sendto_t)(int,
                                 const void*,
                                 size_t,
                                 int,
                                 const struct sockaddr*,
                                 socklen_t);
typedef ssize_t (*real_recvfrom_t)(int fd,
            											 void* restrict buffer,
            											 size_t length,
            											 int flags,
            											 struct sockaddr* restrict address,
            											 socklen_t* restrict address_len);
// clang-format on

typedef int (*real_close_t)(int);

// clang-format off
typedef int (*real_getsockopt_t)
            (int, int, int, void* restrict, socklen_t* restrict);
// clang-format on
typedef int (*real_setsockopt_t)(int, int, int, const void*, socklen_t);
typedef int (*real_getsockname_t)(int, struct sockaddr*, socklen_t*);

/******************** REAL FUNCTIONS ********************/

int real_socket(int domain, int type, int protocol);

ssize_t real_write(int fd, const void* data, size_t size);
ssize_t real_read(int fd, void* data, size_t size);

ssize_t real_send(int fd, const void* buffer, size_t length, int flags);
ssize_t real_recv(int fd, void* buffer, size_t length, int flags);

ssize_t real_sendmsg(int fd, const struct msghdr* message, int flags);
ssize_t real_recvmsg(int fd, struct msghdr* message, int flags);

ssize_t real_sendto(int fd,
										const void* buffer,
										size_t length,
										int flags,
										const struct sockaddr* dest_addr,
										socklen_t dest_len);
ssize_t real_recvfrom(int fd,
											void* restrict buffer,
											size_t length,
											int flags,
											struct sockaddr* restrict address,
											socklen_t* restrict address_len);


int real_accept(int fd, sockaddr* address, socklen_t* length);
int real_connect(int fd, const sockaddr* address, socklen_t length);

int real_getsockopt(int fd,
										int level,
										int option_name,
										void* restrict option_value,
										socklen_t* restrict option_len);

int real_setsockopt(int fd,
										int level,
										int option_name,
										const void* option_value,
										socklen_t option_len);

int real_close(int fd);

/******************** COMMON OVERRIDES ********************/

int getsockopt(int key,
							 int level,
							 int option_name,
							 void* restrict option_value,
							 socklen_t* restrict option_len);

int setsockopt(int key,
							 int level,
							 int option_name,
							 const void* option_value,
							 socklen_t option_len);

#endif /* SOCKET_OVERRIDES_H */
