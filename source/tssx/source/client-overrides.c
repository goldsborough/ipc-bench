#include "tssx/overrides.h"
#include "tssx/selective.h"

int socket(int domain, int type, int protocol) {
	if (socket_is_stream_and_domain(domain, type)) {
		return bridge_generate_key(&bridge);
	} else {
		return socket(domain, type, protocol);
	}
}

void connect(int client_socket, const sockaddr* address, size_t length) {
	Connection connection;
	int check;
	int return_code;

	real_connect(client_socket, address, length);

	if ((check = check_use_tssx(client_socket)) == ERROR) {
		throw("Could not check if socket uses TSSX");
	} else if (!check) {
		return;
	}

	// clang-format off
	return_code = real_read(
		client_socket,
		&connection.segment_id,
		sizeof connection.segment_id
	);
	// clang-format on

	if (return_code == -1) {
		throw("Error receiving segment ID on client side");
	}

	setup_connection(&connection, &DEFAULT_OPTIONS);

	bridge_insert(&bridge, &connection);
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
