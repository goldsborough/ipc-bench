#include "tssx/overrides.h"
#include "tssx/selective.h"

void connect(int client_socket, const sockaddr* address, size_t length) {
	Connection connection;
	int check;
	int return_code;

	real_connect(client_socket, address, length);

	// if ((check = check_use_tssx(client_socket)) == ERROR) {
	// 	throw("Could not check if socket uses TSSX");
	// } else if (!check) {
	// 	return;
	// }

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
	ht_insert(&connection_map, client_socket, &connection);
}

ssize_t read(int socket_fd, void* destination, size_t requested_bytes) {
	// clang-format off
	return connection_read(
		socket_fd,
		destination,
		requested_bytes,
		SERVER_BUFFER
	);
	// clang-format on
}

ssize_t write(int socket_fd, void* source, size_t requested_bytes) {
	// clang-format off
	return connection_write(
		socket_fd,
		source,
		requested_bytes,
		CLIENT_BUFFER
	);
	// clang-format on
}

int close(int socket_fd) {
	Connection* connection;

	connection = ht_get(&connection_map, socket_fd);

	// In case this connection did not use our tssx
	if (connection != NULL) {
		disconnect(connection);
		ht_remove(&connection_map, socket_fd);
	}

	return real_close(socket_fd);
}
