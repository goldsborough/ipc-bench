#include "tssx/client-overrides.h"
#include "tssx/common-overrides.h"
#include "tssx/poll-overrides.h"

int socket(int domain, int type, int protocol) {
	int socket_fd = real_socket(domain, type, protocol);
	if (socket_is_stream_and_domain(domain, type)) {
		// Note: this is no matter if we select the socket to use TSSX or not!
		Session session = {socket_fd, META_STABLE_CONNECTION};
		key_t key = bridge_generate_key(&bridge);
		bridge_insert(&bridge, key, &session);
		return key;
	} else {
		// For internet sockets, UDP sockets etc.
		return socket_fd;
	}
}

int connect(int key, const sockaddr* address, socklen_t length) {
	Session* session;

	if (key < TSSX_KEY_OFFSET) {
		// In this case the key is actually the socket FD
		return real_connect(key, address, length);
	}

	// Lookup the session and stored socket FD
	session = bridge_lookup(&bridge, key);
	if (real_connect(session->socket, address, length) == -1) {
		return ERROR;
	}

	return setup_tssx(session, address);
}

ssize_t read(int key, void* destination, size_t requested_bytes) {
	// clang-format off
	return connection_read(
		key,
		destination,
		requested_bytes,
		SERVER_BUFFER
	);
	// clang-format on
}

ssize_t write(int key, void* source, size_t requested_bytes) {
	// clang-format off
	return connection_write(
		key,
		source,
		requested_bytes,
		CLIENT_BUFFER
	);
	// clang-format on
}

/******************** HELPERS ********************/

int read_segment_id_from_server(int client_socket) {
	int return_code;
	int segment_id;

	// clang-format off
	return_code = real_read(
		client_socket,
		&segment_id,
		sizeof segment_id
	);
	// clang-format on

	if (return_code == -1) {
		throw("Error receiving segment ID on client side");
	}

	return segment_id;
}

int setup_tssx(Session* session, const sockaddr* address) {
	int segment_id;
	int use_tssx;
	ConnectionOptions options;

	if ((use_tssx = client_check_use_tssx(session->socket, address)) == ERROR) {
		print_error("Could not check if socket uses TSSX");
		return ERROR;
	} else if (!use_tssx) {
		assert(session->connection == META_STABLE_CONNECTION);
		session->connection = NULL;
		return SUCCESS;
	}

	// This is for TSSX connections
	segment_id = read_segment_id_from_server(session->socket);
	printf("%d\n", session->socket);
	options = options_from_socket(session->socket, CLIENT);
	session->connection = setup_connection(segment_id, &options);

	return SUCCESS;
}

/******************** "POLYMORPHIC" FUNCTIONS ********************/

void set_non_blocking(Connection* connection, bool non_blocking) {
	connection->server_buffer->timeouts.non_blocking[READ] = non_blocking;
	connection->client_buffer->timeouts.non_blocking[WRITE] = non_blocking;
}

bool get_non_blocking(Connection* connection) {
	assert(connection->server_buffer->timeouts.non_blocking[READ] ==
				 connection->client_buffer->timeouts.non_blocking[WRITE]);
	return connection->client_buffer->timeouts.non_blocking[WRITE];
}

bool ready_for(Connection* connection, Operation operation) {
	if (operation == READ) {
		return !buffer_is_empty(connection->server_buffer);
	} else {
		return !buffer_is_full(connection->client_buffer);
	}
}
