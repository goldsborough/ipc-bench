#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#include "tssx/overrides.h"

int __real_read(int, void*, int);
int __real_write(int, void*, int);
void __real_connect(int, sockaddr*, int*);
int __real_close(int);

void __wrap_connect(int client_socket, sockaddr* address, int* length) {
	Connection connection;
	int return_code;

	__real_connect(client_socket, address, length);

	// clang-format off
	return_code = recv(
		client_socket,
		&connection.segment_id,
		sizeof connection.segment_id,
		0
	);
	// clang-format on

	if (return_code == -1) {
		throw("Error receiving segment ID on client side");
	}

	setup_connection(&connection, &DEFAULT_OPTIONS);

	ht_insert(&connection_map, client_socket, &connection);
}

int __wrap_read(int socket_fd, void* destination, int requested_bytes) {
	// clang-format off
	return connection_read(
		socket_fd,
		destination,
		requested_bytes,
		SERVER_BUFFER
	);
	// clang-format on
}

int __wrap_write(int socket_fd, void* source, int requested_bytes) {
	// clang-format off
	return connection_write(
		socket_fd,
		source,
		requested_bytes,
		CLIENT_BUFFER
	);
	// clang-format on
}

int __wrap_close(int socket_fd) {
	Connection* connection;

	connection = ht_get(&connection_map, socket_fd);
	disconnect(connection);

	return __real_close(socket_fd);
}
