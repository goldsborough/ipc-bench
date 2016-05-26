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

void communicate(int connection, struct Arguments* args) {
	void* buffer = malloc(args->size);

	client_once(NOTIFY);

	for (; args->count > 0; --args->count) {
		// Dummy operation
		memset(buffer, '*', args->size);

		if (write(connection, buffer, args->size) == -1) {
			throw("Error receiving on client-side");
		}

		if (read(connection, buffer, args->size) == -1) {
			throw("Error sending on client-side");
		}
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

	adjust_socket_buffer_size(connection);

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
}

int connect_to_server() {
	// The connection socket
	int connection;

	// Wait until the server created the socket
	client_once(WAIT);

	connection = create_socket();
	connect_socket(connection);

	return connection;
}

int main(int argc, char* argv[]) {
	// The socket through which we communciate with the client
	int connection;

	struct Arguments args;
	parse_arguments(&args, argc, argv);

	connection = connect_to_server();
	communicate(connection, &args);

	return EXIT_SUCCESS;
}
