#ifndef OVERRIDES_H
#define OVERRIDES_H

#include <sys/types.h>

#include "common/utility.h"
#include "tssx/bridge.h"
#include "tssx/connection.h"
#include "tssx/shared_memory.h"

/******************** DEFINITIONS ********************/

#define ERROR -1
#define SERVER_BUFFER 0
#define CLIENT_BUFFER 1

struct HashTable;
struct Connection;
struct Buffer;

typedef struct HashTable HashTable;
typedef struct sockaddr sockaddr;

typedef ssize_t (*real_write_t)(int, const void*, size_t);
typedef ssize_t (*real_read_t)(int, void*, size_t);
typedef int (*real_accept_t)(int, sockaddr*, int*);
typedef void (*real_connect_t)(int, const sockaddr*, int);
typedef int (*real_close_t)(int);
typedef pid_t (*real_fork_t)(void);

/******************** INTERFACE ********************/

ssize_t real_write(int fd, const void* data, size_t size);
ssize_t real_read(int fd, void* data, size_t size);
int real_accept(int fd, sockaddr* address, int* length);
void real_connect(int fd, const sockaddr* address, int length);
int real_close(int fd);
pid_t real_fork(void);

pid_t fork(void);

int connection_write(int key,
										 void* destination,
										 int requested_bytes,
										 int which_buffer);
int connection_read(int key,
										void* source,
										int requested_bytes,
										int which_buffer);

struct Buffer* get_buffer(struct Connection* connection, int which_buffer);

#endif /* OVERRIDES_H */
