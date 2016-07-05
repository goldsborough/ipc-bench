#include <stdatomic.h>


#include <stdio.h>


#include "common/sockets.h"
#include "tssx/buffer.h"
#include "tssx/connection-options.h"
#include "tssx/connection.h"

const ConnectionOptions DEFAULT_OPTIONS =
		DEFAULT_CONNECTION_OPTIONS_INITIALIZER;

ConnectionOptions options_from_socket(int socket_fd, Side side) {
	ConnectionOptions options = DEFAULT_CONNECTION_OPTIONS_INITIALIZER;
	bool non_blocking;
	int server_action;
	int client_action;

	// Not using the socket buffer size because it's way too small (16 KB)
	options.server_buffer_size = DEFAULT_BUFFER_SIZE;
	options.client_buffer_size = DEFAULT_BUFFER_SIZE;

	non_blocking = socket_is_non_blocking(socket_fd);
	server_action = (side == SERVER) ? WRITE : READ;
	client_action = (side == CLIENT) ? WRITE : READ;

	options.server_timeouts.non_blocking[server_action] = non_blocking;
	options.client_timeouts.non_blocking[client_action] = non_blocking;

#ifdef TSSX_SUPPORT_BUFFER_TIMEOUTS
	// clang-format off
	options.server_timeouts.timeout[server_action] = timeout_clocks(
      socket_fd,
      server_action
  );
	options.client_timeouts.timeout[client_action] = timeout_clocks(
      socket_fd,
      client_action
  );
// clang-format on
#endif

	return options;
}

size_t options_segment_size(const ConnectionOptions* options) {
	size_t segment_size = 0;

	segment_size += sizeof(atomic_count_t);
	segment_size += sizeof(Buffer) + options->server_buffer_size;
	segment_size += sizeof(Buffer) + options->client_buffer_size;

	return segment_size;
}

cycle_t timeout_clocks(int socket_fd, Direction direction) {
	double timeout_seconds = socket_timeout_seconds(socket_fd, direction);
	return SECONDS_TO_CLOCKS(timeout_seconds);
}
