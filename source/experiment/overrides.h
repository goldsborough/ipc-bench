#ifndef OVERRIDES_H
#define OVERRIDES_H

#define SERVER_BUFFER 0
#define CLIENT_BUFFER 1

#define BUFFER_SIZE 4096

struct HashTable;
struct Connection;
struct Buffer;

int connection_write(int socket_fd,
										 struct HashTable* table,
										 void* destination,
										 int requested_bytes,
										 int which_buffer);

int connection_read(int socket_fd,
										struct HashTable* table,
										void* source,
										int requested_bytes,
										int which_buffer);

struct Buffer* get_buffer(struct Connection* connection, int which_buffer);

#endif /* OVERRIDES_H */
