#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>

#include "common/sockets.h"
#include "tssx/buffer.h"
#include "tssx/connection.h"
#include "tssx/hashtable.h"
#include "tssx/shared_memory.h"

// clang-format off
ConnectionOptions DEFAULT_OPTIONS = {
	DEFAULT_BUFFER_SIZE,
	DEFAULT_TIMEOUTS_INITIALIZER,
	DEFAULT_BUFFER_SIZE,
	DEFAULT_TIMEOUTS_INITIALIZER
};
// clang-format on

/*************** PUBLIC **************/

void create_connection(Connection* connection, ConnectionOptions* options) {
	void* shared_memory;

	assert(connection != NULL);
	assert(options != NULL);

	connection->segment_id = create_segment(_options_segment_size(options));
	shared_memory = attach_segment(connection->segment_id);

	_init_open_count(connection, shared_memory);
	_create_server_buffer(connection, shared_memory, options);
	_create_client_buffer(connection, shared_memory, options);
}

void setup_connection(Connection* connection, ConnectionOptions* options) {
	void* shared_memory;

	assert(connection != NULL);
	assert(options != NULL);

	shared_memory = attach_segment(connection->segment_id);

	_init_and_increment_open_count(connection, shared_memory);
	_create_server_buffer(connection, shared_memory, options);
	_create_client_buffer(connection, shared_memory, options);
}

void connection_add_user(Connection* connection) {
	assert(connection != NULL);
	atomic_fetch_add(connection->open_count, 1);
}

void disconnect(Connection* connection) {
	assert(connection != NULL);

	atomic_fetch_sub(connection->open_count, 1);

	if (atomic_load(connection->open_count) == 0) {
		_detach_connection(connection);
		_destroy_connection(connection);
	} else {
		_detach_connection(connection);
	}
}

/*************** UTILITY **************/

void _create_server_buffer(Connection* connection,
													 void* shared_memory,
													 ConnectionOptions* options) {
	shared_memory += sizeof(atomic_count_t);
	// clang-format off
	connection->server_buffer = create_buffer(
			shared_memory,
			options->server_buffer_size,
			&options->server_timeouts
	);
	// clang-format on
}

void _create_client_buffer(Connection* connection,
													 void* shared_memory,
													 ConnectionOptions* options) {
	shared_memory += sizeof(atomic_count_t);
	shared_memory += segment_size(connection->server_buffer);
	// clang-format off
	connection->client_buffer = create_buffer(
			shared_memory,
			options->client_buffer_size,
			&options->client_timeouts
	);
	// clang-format on
}

void _init_open_count(Connection* connection, void* shared_memory) {
	connection->open_count = (atomic_count_t*)shared_memory;
	atomic_init(connection->open_count, 1);
}

void _init_and_increment_open_count(Connection* connection,
																		void* shared_memory) {
	connection->open_count = (atomic_count_t*)shared_memory;
	atomic_fetch_add(connection->open_count, 1);
}

void _detach_connection(Connection* connection) {
	detach_segment(_segment_start(connection));
}

void _destroy_connection(Connection* connection) {
	destroy_segment(connection->segment_id);
}

void* _segment_start(Connection* connection) {
	return (void*)connection->open_count;
}

int _options_segment_size(ConnectionOptions* options) {
	int segment_size;

	segment_size += sizeof(atomic_count_t);
	segment_size += sizeof(Buffer) + options->server_buffer_size;
	segment_size += sizeof(Buffer) + options->client_buffer_size;

	return segment_size;
}

int _connection_segment_size(Connection* connection) {
	int segment_size;

	segment_size += sizeof(atomic_count_t);
	segment_size += sizeof(Buffer) + connection->server_buffer->size;
	segment_size += sizeof(Buffer) + connection->client_buffer->size;

	return segment_size;
}
