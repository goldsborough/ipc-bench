#ifndef OVERRIDES_H
#define OVERRIDES_H

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

extern HashTable connection_map;

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
