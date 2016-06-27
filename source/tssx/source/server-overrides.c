#include <assert.h>
#include <sys/un.h>

#include "tssx/overrides.h"
#include "tssx/selective.h"

int accept(int server_socket, sockaddr* address, int* length) {
	Connection connection;
	int client_socket;
	int check;
	int return_code;

	if ((client_socket = real_accept(server_socket, address, length)) == ERROR) {
		return ERROR;
	}

	if ((check = check_use_tssx(server_socket)) == ERROR) {
		return ERROR;
	} else if (!check) {
		return client_socket;
	}

	create_connection(&connection, &DEFAULT_OPTIONS);

	// clang-format off
	return_code = real_write(
		client_socket,
		&connection.segment_id,
		sizeof connection.segment_id
	);
	// clang-format on

	if (return_code == ERROR) return ERROR;

	return bridge_insert(&bridge, &connection);
}

ssize_t read(int key, void* destination, size_t requested_bytes) {
	// clang-format off
	return connection_read(
		key,
		destination,
		requested_bytes,
		CLIENT_BUFFER
	);
	// clang-format on
}

ssize_t write(int key, void* source, size_t requested_bytes) {
	// clang-format off
	return connection_write(
		key,
		source,
		requested_bytes,
		SERVER_BUFFER
	);
	// clang-format on
}

int close(int key) {
	Connection* connection;

	if (key < 0) {
		connection = bridge_lookup(&bridge, key);
		assert(connection != NULL);
		disconnect(connection);
		bridge_remove(&bridge, key);
	}

	return real_close(key);
}
