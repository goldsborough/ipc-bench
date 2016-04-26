#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>

#include "common/common.h"
#include "common/sockets.h"

#define SOCKET_PATH "/tmp/ipc_bench_socket"

void cleanup(int connection, void* buffer) {
	close(connection);
	free(buffer);
	remove(SOCKET_PATH);
}

void communicate(int connection, struct Arguments* args) {
	struct Benchmarks bench;
	int message;
	void* buffer;

	buffer = malloc(args->count);
	setup_benchmarks(&bench);

	for (message = 0; message < args->count; ++message) {
		bench.single_start = now();

		if (send(connection, buffer, args->size, 0) < args->size) {
			throw("Error sending on server-side");
		}

		if (recv(connection, buffer, args->size, 0) < args->size) {
			throw("Error receiving on server-side");
		}

		benchmark(&bench);
	}

	evaluate(&bench, args);
	cleanup(connection, buffer);
}

void setup_socket(int socket_descriptor, struct Arguments* args) {
	int return_code;

	// The main datastructure for a UNIX-domain socket.
	// It only has two members:
	// 1. sun_family: The family of the socket. Should be AF_UNIX
	//                for UNIX-domain sockets.
	// 2. sun_path: Noting that a UNIX-domain socket ist just a
	//              file in the file-system, it also has a path.
	//              This may be any path the program has permission
	//              to create, read and write files in. The maximum
	//              size of such a path is 108 bytes.
	struct sockaddr_un address;

	// Let's first adjust the buffer size
	adjust_socket_buffer_size(socket_descriptor, args->size);

	// Set the family of the address strct
	address.sun_family = AF_UNIX;
	// Copy in the path
	strcpy(address.sun_path, SOCKET_PATH);
	// Remove the socket if it already exists
	remove(address.sun_path);

	// Bind the socket to an address.
	// Arguments:
	// 1. The socket file-descriptor.
	// 2. A sockaddr struct, which we get by casting our address struct.
	// 3. The length of the struct, as computed by the SUN_LEN macro.
	// clang-format off
	return_code = bind(
		socket_descriptor,
		(struct sockaddr*)&address,
		SUN_LEN(&address)
	);
	// clang-format on

	if (return_code == -1) {
		throw("Error binding socket to address");
	}

	return_code = listen(socket_descriptor, 10);

	if (return_code == -1) {
		throw("Could not start listening on socket");
	}
}

int create_socket(struct Arguments* args) {
	// File descriptor for the socket
	int socket_descriptor;

	// Get a new socket from the OS
	// Arguments:
	// 1. The family of the socket (AF_UNIX for UNIX-domain sockets)
	// 2. The socket type, either stream-oriented (TCP) or
	//    datagram-oriented (UDP)
	// 3. The protocol for the given socket type. By passing 0, the
	//    OS will pick the right protocol for the job (TCP/UDP)
	socket_descriptor = socket(AF_UNIX, SOCK_STREAM, 0);

	if (socket_descriptor == -1) {
		throw("Error opening socket on server-side");
	}

	setup_socket(socket_descriptor, args);

	// Notify the client that it can connect to the socket now
	server_once(NOTIFY);

	return socket_descriptor;
}
int accept_connection(int socket_descriptor) {
	struct sockaddr_un client;
	int connection;
	socklen_t length = sizeof client;

	// clang-format off
	connection = accept(
		socket_descriptor,
		(struct sockaddr*)&client,
		&length
	);
	// clang-format on

	if (connection == -1) {
		throw("Error accepting connection");
	}

	// Don't need this one anymore (because we only have one connection)
	close(socket_descriptor);

	return connection;
}

int main(int argc, char* argv[]) {
	// File descriptor for the server socket
	int socket_descriptor;
	// File descriptor for the socket over which
	// the communciation will happen with the client
	int connection;
	// For command-line arguments
	struct Arguments args;
	parse_arguments(&args, argc, argv);

	socket_descriptor = create_socket(&args);
	connection = accept_connection(socket_descriptor);

	communicate(connection, &args);

	return EXIT_SUCCESS;
}
