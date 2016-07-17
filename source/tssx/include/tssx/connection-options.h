#ifndef CONNECTION_OPTIONS_H
#define CONNECTION_OPTIONS_H

#include <stdbool.h>
#include <stddef.h>

#include "tssx/definitions.h"
#include "tssx/timeouts.h"

/******************** DEFINITIONS ********************/

#define DEFAULT_BUFFER_SIZE 1000000

// clang-format off
#define DEFAULT_CONNECTION_OPTIONS_INITIALIZER \
{ \
	DEFAULT_BUFFER_SIZE, \
	DEFAULT_TIMEOUTS_INITIALIZER, \
	DEFAULT_BUFFER_SIZE, \
	DEFAULT_TIMEOUTS_INITIALIZER, \
   true \
}
// clang-format on

/******************** STRUCTURES ********************/

typedef struct ConnectionOptions {
	size_t server_buffer_size;
	Timeouts server_timeouts;

	size_t client_buffer_size;
	Timeouts client_timeouts;

	// Is the connection blocking ?
	bool isBlocking;

} ConnectionOptions;

extern const ConnectionOptions DEFAULT_OPTIONS;

/******************** INTERFACE ********************/

ConnectionOptions options_from_socket(int socket_fd, Side side);
size_t options_segment_size(const ConnectionOptions* options);

/******************** PRIVATE ********************/

enum Direction;
cycle_t socket_timeout_clocks(int socket_fd, enum Direction direction);

#endif /* CONNECTION_OPTIONS_H */
