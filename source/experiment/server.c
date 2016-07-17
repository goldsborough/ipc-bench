#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "common/common.h"
#include "common/sockets.h"

#define SOCKET_PATH "/tmp/domain_socket"

void cleanup(int connection, void* buffer) {
	close(connection);
	free(buffer);
	if (remove(SOCKET_PATH) == -1) {
		throw("Error removing domain socket");
	}
}

#include <x86intrin.h>
clock_t t_now() {
	return __rdtsc();
}

void communicate(int connection, Arguments* args) {
	Benchmarks bench;
	int message;
	void* buffer;
	// clock_t start;

	buffer = malloc(args->size);
	setup_benchmarks(&bench);

	// Synchronize for the benchmarks
	// server_once(WAIT);
	if (read(connection, buffer, 1) < 1) {
		throw("Error reading first byte from client");
	}

	for (message = 0; message < args->count; ++message) {
		bench.single_start = now();

		// printf("Server start\n");
		// clock_t start = t_now();

		if (read(connection, buffer, args->size) != args->size) {
			throw("Error reading on server-side");
		}

		// printf("Server read done %llu\n", t_now() - start);

		memset(buffer, '*', args->size);

		// start = t_now();
		// printf("Server write done %llu\n", t_now() - start);
		// start = t_now();
		if (write(connection, buffer, args->size) != args->size) {
			throw("Error sending on server-side");
		}

		// printf("Server write done %llu\n", t_now() - start);

		benchmark(&bench);
	}

	evaluate(&bench, args);
	cleanup(connection, buffer);
}

void setup_socket_address(struct sockaddr_un* address) {
	address->sun_family = AF_UNIX;
	strcpy(address->sun_path, SOCKET_PATH);

	// Remove it if it already exists (will throw EINVAL
	// error if it already exists and we try to create it)
	unlink(address->sun_path);
}

void setup_socket(int server_socket) {
	int return_code;
	struct sockaddr_un address;

	setup_socket_address(&address);

	// clang-format off
	return_code = bind(
		server_socket,
		(struct sockaddr*) &address,
		SUN_LEN(&address)
	);
	// clang-format on

	if (return_code == -1) {
		throw("Error binding socket to address!");
	}

	// Start listening on the socket (max 10 queueing)
	return_code = listen(server_socket, 10);

	if (return_code == -1) {
		throw("Could not start listening on socket");
	}
}

int create_server_socket() {
	// The socket on which the server receives
	// all incoming connections
	int server_socket;

	if ((server_socket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		throw("Error opening server-socket on server-side");
	}

	setup_socket(server_socket);

	// server_once(NOTIFY);

	return server_socket;
}

int accept_client(int server_socket) {
	int client_socket;
	struct sockaddr_un client_address;
	socklen_t length = sizeof client_socket;

	// clang-format off
	client_socket = accept(
		server_socket,
		(struct sockaddr*)&client_address,
		&length
	);
	// clang-format on

	if (client_socket == -1) {
		throw("Error accepting connection");
	}

	set_socket_both_buffer_sizes(client_socket);

	return client_socket;
}

int connect_to_client() {
	// The socket on which the server receives
	// all incoming connections
	int server_socket;
	// The socket for unique communication with the client
	int client_socket;

	server_socket = create_server_socket();
	client_socket = accept_client(server_socket);

	// Don't need the server socket anymore (only have one connection)
	// close(client_socket);

	if (fork() != (pid_t)0) {
		// Don't need the server socket anymore (only have one connection)
		close(client_socket);
		close(server_socket);
		exit(EXIT_SUCCESS);
	}

	return client_socket;
}

int main(int argc, char* argv[]) {
	// The socket through which we communicate with the client
	int connection;

	Arguments args;
	parse_arguments(&args, argc, argv);

	connection = connect_to_client();
	communicate(connection, &args);

	return EXIT_SUCCESS;
}
