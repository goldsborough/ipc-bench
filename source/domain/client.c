#include <fcntl.h>
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

void communicate(int connection, struct Arguments* args, int busy_waiting) {
	void* buffer = malloc(args->size);

	for (; args->count > 0; --args->count) {
		if (receive(connection, buffer, args->size, busy_waiting) == -1) {
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

void setup_socket(int connection, int busy_waiting) {
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

	set_socket_both_buffer_sizes(connection);

	if (busy_waiting) {
		// For domain sockets blocking or not seems to make no
		// difference at all in terms of speed. Neither setting
		// the timeout nor not blocking at all.
		// adjust_socket_blocking_timeout(connection, 0, 1);
		if (set_io_flag(connection, O_NONBLOCK) == -1) {
			throw("Error setting socket to non-blocking on client-side");
		}
	}

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
	// Blocks until the connection is accepted by the other end.
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

int create_connection(int busy_waiting) {
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

	setup_socket(connection, busy_waiting);

	return connection;
}

int main(int argc, char* argv[]) {
	// File descriptor for the socket over which
	// the communciation will happen with the client
	int connection;

	// Flag to determine whether or not to
	// do busy-waiting and non-blocking calls
	int busy_waiting;

	// For command-line arguments
	struct Arguments args;

	busy_waiting = check_flag("busy", argc, argv);
	parse_arguments(&args, argc, argv);

	connection = create_connection(busy_waiting);
	communicate(connection, &args, busy_waiting);

	return EXIT_SUCCESS;
}
