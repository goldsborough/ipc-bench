#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>

#include "common/common.h"
#include "common/sockets.h"

#define SOCKET_PATH "/tmp/domain_socket"

void cleanup(int socket, void* buffer) {
	close(socket);
	free(buffer);
}

#include <x86intrin.h>
clock_t t_now() {
	return __rdtsc();
}

void communicate(int connection, Arguments* args) {
	void* buffer = malloc(args->size);

	// Benchmarks can start
	// client_once(NOTIFY);
	if (write(connection, buffer, 1) < 1) {
		throw("Error writing first byte to server");
	}

	for (; args->count > 0; --args->count) {
		// Dummy operation
		memset(buffer, '*', args->size);

		// printf("Client start\n");
		// clock_t start = t_now();

		if (write(connection, buffer, args->size) == -1) {
			throw("Error writing on client-side");
		}

		// printf("Client write done %llu\n", t_now() - start);

		// start = t_now();

		if (read(connection, buffer, args->size) == -1) {
			throw("Error reading on client-side");
		}

		// printf("Client read  done %llu\n", t_now() - start);
	}

	cleanup(connection, buffer);
}

void setup_socket_address(struct sockaddr_un* address) {
	address->sun_family = AF_UNIX;
	strcpy(address->sun_path, SOCKET_PATH);
}

int create_socket() {
	int connection;

	connection = socket(AF_UNIX, SOCK_STREAM, 0);

	if (connection == -1) {
		throw("Error creating socket on client-side");
	}

	set_socket_both_buffer_sizes(connection);

	return connection;
}

void connect_socket(int connection) {
	int return_code;
	struct sockaddr_un server_address;

	setup_socket_address(&server_address);

	// clang-format off
	return_code = connect(
		connection,
		(struct sockaddr*)&server_address,
		SUN_LEN(&server_address)
	);
	// clang-format on

	if (return_code == -1) {
		throw("Error connecting to server");
	}

	// set_socket_non_blocking(connection);
}

int connect_to_server() {
	// The connection socket
	int connection;

	// Wait until the server created the socket
	// client_once(WAIT);

	connection = create_socket();
	connect_socket(connection);

	return connection;
}

int main(int argc, char* argv[]) {
	// The socket through which we communicate with the client
	int connection;

	Arguments args;
	parse_arguments(&args, argc, argv);

	connection = connect_to_server();
	communicate(connection, &args);

	return EXIT_SUCCESS;
}
