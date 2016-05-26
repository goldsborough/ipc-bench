#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#include "tssx/overrides.h"

typedef struct sockaddr sockaddr;

void __real_connect(int, sockaddr*, int*);
int __real_read(int, void*, int);
int __real_write(int, void*, int);

void __wrap_connect(int client_socket, sockaddr* address, int* length) {
	Connection connection;
	int return_code;

	__real_connect(client_socket, address, length);

	printf("??\n");

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

	setup_connection(&connection, BUFFER_SIZE);

	ht_insert(&connection_map, client_socket, &connection);
}

int __wrap_read(int socket_fd, void* destination, int requested_bytes) {
	// clang-format off
	return connection_read(
		socket_fd,
		&connection_map,
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
		&connection_map,
		source,
		requested_bytes,
		CLIENT_BUFFER
	);
	// clang-format on
}
