#include <assert.h>
#include <sys/un.h>

#include "tssx/overrides.h"

int accept(int server_socket, sockaddr* address, int* length) {
	Connection connection;
	int client_socket;
	int return_code;

	printf("???\n");

	client_socket = real_accept(server_socket, address, length);

	printf("Address: %s\n", ((struct sockaddr_un*)address)->sun_path);

	if (client_socket == -1) return -1;

	connection.segment_id =
			create_segment(options_segment_size(&DEFAULT_OPTIONS));

	// clang-format off
	return_code = real_write(
		client_socket,
		&connection.segment_id,
		sizeof connection.segment_id
	);
	// clang-format on

	if (return_code == -1) {
		throw("Error sending segment ID to client");
		return -1;
	}

	setup_connection(&connection, &DEFAULT_OPTIONS);

	ht_insert(&connection_map, client_socket, &connection);

	return client_socket;
}

ssize_t read(int socket_fd, void* destination, size_t requested_bytes) {
	// clang-format off
	return connection_read(
		socket_fd,
		destination,
		requested_bytes,
		CLIENT_BUFFER
	);
	// clang-format on
}

ssize_t write(int socket_fd, void* source, size_t requested_bytes) {
	// clang-format off
	return connection_write(
		socket_fd,
		source,
		requested_bytes,
		SERVER_BUFFER
	);
	// clang-format on
}

int close(int socket_fd) {
	Connection* connection;

	connection = ht_get(&connection_map, socket_fd);

	// Not all socket FDs will be associated with a connection
	// for example the server socket is not (only the server-client
	// communication socket)
	if (connection != NULL) {
		destroy_connection(connection);
		ht_remove(&connection_map, socket_fd);
	}

	return real_close(socket_fd);
}
