#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#include "tssx/buffer.h"
#include "tssx/hashtable.h"
#include "tssx/overrides.h"

HashTable connection_map = HT_INITIALIZER;

int connection_write(int socket_fd,
										 void* destination,
										 int requested_bytes,
										 int which_buffer) {
	Connection* connection;
	int bytes_written;

	connection = ht_get(&connection_map, socket_fd);
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
										void* source,
										int requested_bytes,
										int which_buffer) {
	Connection* connection;
	int bytes_read;

	connection = ht_get(&connection_map, socket_fd);
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

Buffer* get_buffer(Connection* connection, int which_buffer) {
	return which_buffer ? connection->client_buffer : connection->server_buffer;
}
