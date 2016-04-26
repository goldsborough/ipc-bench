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
}

void communicate(int connection, struct Arguments* args) {
	void* buffer = malloc(args->size);

	for (; args->count > 0; --args->count) {
		if (recv(connection, buffer, args->size, 0) == -1) {
			throw("Error receiving on client-side");
		}

		// Dummy operation
		memset(buffer, '*', args->size);

		if (send(connection, buffer, args->size, 0) == -1) {
			throw("Error sending on client-side");
		}
	}

	cleanup(connection, buffer);
}

void setup_socket(int connection, struct Arguments* args) {
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

	adjust_socket_buffer_size(connection, args->size);

	// Set the family of the address struct
	address.sun_family = AF_UNIX;
	// Copy in the path
	strcpy(address.sun_path, SOCKET_PATH);

	// Connect the socket to an address.
	// Arguments:
	// 1. The socket file-descriptor.
	// 2. A sockaddr struct describing the socket address to connect to.
	// 3. The length of the struct, as computed by the SUN_LEN macro.
	// clang-format off
	return_code = connect(
		connection,
		(struct sockaddr*)&address,
		SUN_LEN(&address)
	);
	// clang-format on

	if (return_code == -1) {
		throw("Error connecting to server");
	}
}

int create_connection(struct Arguments* args) {
	// The connection socket (file descriptor) that we will return
	int connection;

	// Wait until the server is listening on the socket
	client_once(WAIT);

	// Get a new socket from the OS
	// Arguments:
	// 1. The family of the socket (AF_UNIX for UNIX-domain sockets)
	// 2. The socket type, either stream-oriented (TCP) or
	//    datagram-oriented (UDP)
	// 3. The protocol for the given socket type. By passing 0, the
	//    OS will pick the right protocol for the job (TCP/UDP)
	connection = socket(AF_UNIX, SOCK_STREAM, 0);

	if (connection == -1) {
		throw("Error opening socket on client-side");
	}

	setup_socket(connection, args);

	return connection;
}

int main(int argc, char* argv[]) {
	// File descriptor for the socket over which
	// the communciation will happen with the client
	int connection;
	// For command-line arguments
	struct Arguments args;
	parse_arguments(&args, argc, argv);

	connection = create_connection(&args);
	communicate(connection, &args);

	return EXIT_SUCCESS;
}
