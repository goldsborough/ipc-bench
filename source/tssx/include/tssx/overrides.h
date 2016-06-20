#ifndef OVERRIDES_H
#define OVERRIDES_H

#include <sys/types.h>

#include "common/utility.h"
#include "tssx/connection.h"
#include "tssx/hashtable.h"
#include "tssx/shared_memory.h"

#define SERVER_BUFFER 0
#define CLIENT_BUFFER 1

struct HashTable;
struct Connection;
struct Buffer;

typedef struct HashTable HashTable;
typedef struct Connection Connection;
typedef struct sockaddr sockaddr;
typedef struct Buffer Buffer;

typedef ssize_t (*real_write_t)(int, const void*, size_t);
typedef ssize_t (*real_read_t)(int, void*, size_t);
typedef int (*real_accept_t)(int, sockaddr*, int*);
typedef void (*real_connect_t)(int, const sockaddr*, int);
typedef int (*real_close_t)(int);

extern HashTable connection_map;

ssize_t real_write(int fd, const void* data, size_t size);
ssize_t real_read(int fd, void* data, size_t size);
int real_accept(int fd, sockaddr* address, int* length);
void real_connect(int fd, const sockaddr* address, int length);
int real_close(int fd);


int connection_write(int socket_fd,
										 void* destination,
										 int requested_bytes,
										 int which_buffer);
int connection_read(int socket_fd,
										void* source,
										int requested_bytes,
										int which_buffer);

Buffer* get_buffer(Connection* connection, int which_buffer);

#endif /* OVERRIDES_H */
