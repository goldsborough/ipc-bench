#include "tssx/server-overrides.h"
#include "tssx/common-overrides.h"
#include "tssx/poll-overrides.h"

#include <sys/socket.h>
#include <sys/types.h>

/******************** OVERRIDES ********************/

int accept(int server_socket, sockaddr *address, socklen_t *length) {
	int client_socket;
	int use_tssx;

	if ((client_socket = real_accept(server_socket, address, length)) == ERROR) {
		return ERROR;
	}

	if ((use_tssx = check_tssx_usage(server_socket, SERVER)) == ERROR) {
		return ERROR;
	} else if (!use_tssx) {
		return client_socket;
	}

	if (setup_tssx(client_socket) == ERROR) {
		return ERROR;
	}

	return client_socket;
}

ssize_t read(int key, void *destination, size_t requested_bytes) {
	// clang-format off
	return connection_read(
		key,
		destination,
		requested_bytes,
		CLIENT_BUFFER
	);
	// clang-format on
}

ssize_t write(int key, const void *source, size_t requested_bytes) {
	// clang-format off
	return connection_write(
		key,
		source,
		requested_bytes,
		SERVER_BUFFER
	);
	// clang-format on
}

/******************** HELPERS ********************/

int send_segment_id_to_client(int client_socket, Session *session) {
	int return_code;

	// clang-format off
	return_code = real_write(
		client_socket,
		&session->connection->segment_id,
		sizeof session->connection->segment_id
	);
	// clang-format on

	if (return_code == ERROR) {
		print_error("Error sending segment ID to client");
		disconnect(session->connection);
		return ERROR;
	}

	return return_code;
}

int setup_tssx(int client_socket) {
	Session session;
	ConnectionOptions options;

	options = options_from_socket(client_socket, SERVER);
	session.connection = create_connection(&options);

	if (session.connection == NULL) {
		print_error("Error allocating connection resources");
		return ERROR;
	}

	if (send_segment_id_to_client(client_socket, &session) == ERROR) {
		print_error("Error sending segment ID to client");
		return ERROR;
	}

	bridge_insert(&bridge, client_socket, &session);

	return SUCCESS;
}

/******************** "POLYMORPHIC" FUNCTIONS ********************/

void set_non_blocking(Connection *connection, bool non_blocking) {
	connection->server_buffer->timeouts.non_blocking[WRITE] = non_blocking;
	connection->client_buffer->timeouts.non_blocking[READ] = non_blocking;
}

bool is_non_blocking(Connection *connection) {
	assert(connection->server_buffer->timeouts.non_blocking[WRITE] ==
				 connection->client_buffer->timeouts.non_blocking[READ]);
	return connection->server_buffer->timeouts.non_blocking[WRITE];
}

bool _ready_for(Connection *connection, Operation operation) {
	if (operation == READ) {
		return buffer_ready_for(connection->client_buffer, READ);
	} else {
		return buffer_ready_for(connection->server_buffer, WRITE);
	}
}
