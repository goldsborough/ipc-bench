#include <stdio.h>
#include <assert.h>

#include "tssx/overrides.h"
#include "tssx/buffer.h"

HashTable connection_map = HT_INITIALIZER;


int connection_write(int socket_fd,
										 void* destination,
										 int requested_bytes,
										 int which_buffer) {
	Connection* connection;

	connection = ht_get(&connection_map, socket_fd);
	assert(connection != NULL);

	// clang-format off
	return buffer_write(
		get_buffer(connection, which_buffer),
		destination,
		requested_bytes
	);
	// clang-format on
}

int connection_read(int socket_fd,
										void* source,
										int requested_bytes,
										int which_buffer) {
	Connection* connection;

	connection = ht_get(&connection_map, socket_fd);
	assert(connection != NULL);

	// clang-format off
	return buffer_read(
		get_buffer(connection, which_buffer),
		source,
		requested_bytes
	);
	// clang-format on
}

Buffer* get_buffer(Connection* connection, int which_buffer) {
	return which_buffer ? connection->client_buffer : connection->server_buffer;
}
