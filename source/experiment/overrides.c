#include <assert.h>
#include <stddef.h>

#include "buffer.h"
#include "hashtable.h"
#include "overrides.h"

int connection_write(int socket_fd,
										 struct HashTable* table,
										 void* destination,
										 int requested_bytes,
										 int which_buffer) {
	Connection* connection;
	int bytes_written;

	connection = ht_get(table, socket_fd);
	assert(connection != NULL);

	// clang-format off
	bytes_written = buffer_write(
		get_buffer(connection, which_buffer),
		destination,
		requested_bytes
	);
	// clang-format on

	return bytes_written;
}

int connection_read(int socket_fd,
										struct HashTable* table,
										void* source,
										int requested_bytes,
										int which_buffer) {
	Connection* connection;
	int bytes_read;

	connection = ht_get(table, socket_fd);
	assert(connection != NULL);

	// clang-format off
	bytes_read = buffer_read(
		get_buffer(connection, which_buffer),
		source,
		requested_bytes
	);
	// clang-format on

	return bytes_read;
}

struct Buffer* get_buffer(struct Connection* connection, int which_buffer) {
	return which_buffer ? connection->client_buffer : connection->server_buffer;
}
