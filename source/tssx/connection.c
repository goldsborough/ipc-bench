#include <assert.h>
#include <stddef.h>

#include "common/sockets.h"
#include "tssx/buffer.h"
#include "tssx/connection.h"
#include "tssx/shared_memory.h"

// clang-format off
ConnectionOptions DEFAULT_OPTIONS = {
	DEFAULT_BUFFER_SIZE,
	DEFAULT_TIMEOUT,
	DEFAULT_BUFFER_SIZE,
	DEFAULT_TIMEOUT
};
// clang-format on

void setup_connection(Connection* connection, ConnectionOptions* options) {
	void* shared_memory;

	assert(connection != NULL);
	assert(options != NULL);

	shared_memory = attach_segment(connection->segment_id);

	create_server_buffer(connection, shared_memory, options);
	create_client_buffer(connection, shared_memory, options);
}

void server_options_from_socket(ConnectionOptions* options, int socket_fd) {
	options->server_buffer_size = get_socket_buffer_size(socket_fd, SEND);
	options->client_buffer_size = get_socket_buffer_size(socket_fd, RECEIVE);

	options->server_timeout = get_socket_timeout_seconds(socket_fd, SEND);
	options->client_timeout = get_socket_timeout_seconds(socket_fd, RECEIVE);
}
void client_options_from_socket(ConnectionOptions* options, int socket_fd) {
	options->server_buffer_size = get_socket_buffer_size(socket_fd, RECEIVE);
	options->client_buffer_size = get_socket_buffer_size(socket_fd, SEND);

	options->server_timeout = get_socket_timeout_seconds(socket_fd, RECEIVE);
	options->client_timeout = get_socket_timeout_seconds(socket_fd, SEND);
}

void destroy_connection(Connection* connection) {
	assert(connection != NULL);

	disconnect(connection);
	destroy_segment(connection->segment_id);
}

void disconnect(Connection* connection) {
	// The segment starts at the server_buffer pointer
	detach_segment(connection->server_buffer);
}

void create_server_buffer(Connection* connection,
													void* shared_memory,
													ConnectionOptions* options) {
	// clang-format off
	connection->server_buffer = create_buffer(
			shared_memory,
			options->server_buffer_size,
			options->server_timeout
	);
	// clang-format on
}

void create_client_buffer(Connection* connection,
													void* shared_memory,
													ConnectionOptions* options) {
	shared_memory += segment_size(connection->server_buffer);
	// clang-format off
	connection->client_buffer = create_buffer(
			shared_memory,
			options->client_buffer_size,
			options->client_timeout
	);
	// clang-format on
}
