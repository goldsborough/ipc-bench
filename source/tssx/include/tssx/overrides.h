#ifndef OVERRIDES_H
#define OVERRIDES_H

#include <sys/types.h>

#include "common/utility.h"
#include "tssx/bridge.h"
#include "tssx/connection.h"
#include "tssx/shared-memory.h"

/******************** DEFINITIONS ********************/

#define ERROR -1
#define SUCCESS 0
#define SERVER_BUFFER 0
#define CLIENT_BUFFER 1

struct HashTable;
struct Connection;
struct Buffer;

typedef struct HashTable HashTable;
typedef struct sockaddr sockaddr;

typedef unsigned int socklen_t;

// Socket API
typedef int (*real_socket_t)(int, int, int);
typedef ssize_t (*real_write_t)(int, const void*, size_t);
typedef ssize_t (*real_read_t)(int, void*, size_t);
typedef int (*real_accept_t)(int, sockaddr*, socklen_t*);
typedef int (*real_connect_t)(int, const sockaddr*, socklen_t);
typedef int (*real_close_t)(int);

// clang-format off
typedef int (*real_getsockopt_t)
            (int, int, int, void* restrict, socklen_t* restrict);
// clang-format on
typedef int (*real_setsockopt_t)(int, int, int, const void*, socklen_t);

// Other
typedef pid_t (*real_fork_t)(void);

/******************** REAL FUNCTIONS ********************/

// Socket API
int real_socket(int domain, int type, int protocol);

ssize_t real_write(int fd, const void* data, size_t size);
ssize_t real_read(int fd, void* data, size_t size);

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

// Other
pid_t real_fork(void);

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

pid_t fork(void);

/******************** INTERFACE ********************/

int connection_write(int key,
										 void* destination,
										 int requested_bytes,
										 int which_buffer);
int connection_read(int key,
										void* source,
										int requested_bytes,
										int which_buffer);

struct Buffer* get_buffer(struct Connection* connection, int which_buffer);

int socket_is_stream_and_domain(int domain, int type);

#endif /* OVERRIDES_H */
