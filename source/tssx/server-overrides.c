#include <assert.h>
#include <sys/socket.h>
#include <unistd.h>

#include "tssx/overrides.h"

int __real_read(int, void*, int);
int __real_write(int, void*, int);
int __real_accept(int, sockaddr*, int*);
int __real_close(int);

int __wrap_accept(int server_socket, sockaddr* address, int* length) {
	Connection connection;
	int client_socket;
	int return_code;

	client_socket = __real_accept(server_socket, address, length);

	if (client_socket == -1) return -1;

	connection.segment_id = create_segment(BUFFER_SIZE);

	// clang-format off
	return_code = send(
		client_socket,
		&connection.segment_id,
		sizeof connection.segment_id,
		0
	);
	// clang-format on

	if (return_code == -1) {
		throw("Error sending segment ID to client");
		return -1;
	}

	setup_connection(&connection, BUFFER_SIZE);

	ht_insert(&connection_map, client_socket, &connection);

	return client_socket;
}

int __wrap_read(int socket_fd, void* destination, int requested_bytes) {
	// clang-format off
	return connection_read(
		socket_fd,
		destination,
		requested_bytes,
		CLIENT_BUFFER
	);
	// clang-format on
}

int __wrap_write(int socket_fd, void* source, int requested_bytes) {
	// clang-format off
	return connection_write(
		socket_fd,
		source,
		requested_bytes,
		SERVER_BUFFER
	);
	// clang-format on
}

int __wrap_close(int socket_fd) {
	Connection* connection;

	connection = ht_get(&connection_map, socket_fd);
	destroy_connection(connection);

	return __real_close(socket_fd);
}
