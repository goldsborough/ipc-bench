#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common/common.h"
#include "common/sockets.h"

#define PORT "6969"
#define HOST "localhost"

void handle_blocking(int socket_descriptor) {
	// Will be necessary when calling setsockopt to free busy sockets
	int yes = 1;

	// Reclaim blocked but unused sockets
	// (from zombie processes)
	// clang-format off
	int return_code = setsockopt(
		socket_descriptor,
		SOL_SOCKET,
		SO_REUSEADDR,
		&yes,
		sizeof yes
	 );
	// clang-format on

	// If the socket is blocked, tell the OS to reclaim it
	if (return_code == -1) {
		throw("Error reclaiming socket!\n");
	}
}

struct addrinfo *
get_address(struct addrinfo *server_info, int *socket_descriptor) {
	struct addrinfo *valid_address;
	int return_code;

	// Iterate through the address linked-list until
	// we find one we can get a socket for
	for (valid_address = server_info; valid_address != NULL;
			 valid_address = valid_address->ai_next) {
		// The call to socket() is the first step in establishing a socket
		// based communication. It takes the following arguments:
		// 1. The address family (PF_INET or PF_INET6)
		// 2. The socket type (SOCK_STREAM or SOCK_DGRAM)
		// 3. The protocol (TCP or UDP)
		// Note that all of these fields will have been already populated
		// by getaddrinfo. If the call succeeds, it returns a valid file descriptor.
		// clang-format off
		*socket_descriptor = socket(
			valid_address->ai_family,
			valid_address->ai_socktype,
			valid_address->ai_protocol
		 );
		// clang-format on

		if (*socket_descriptor == -1) {
			continue;
		}

		handle_blocking(*socket_descriptor);

		// Once we have a socket, we need to bind it to an address.
		// Again, this information we get from the addrinfo struct
		// that was populated by getaddrinfo(). The arguments are:
		// 1. The socket file_descriptor to which to bind the address.
		// 2. The address to bind (sockaddr_in struct)
		// 3. The size of this address structure.
		// clang-format off
		return_code = bind(
			*socket_descriptor,
			valid_address->ai_addr,
			valid_address->ai_addrlen
		);
		// clang-format on

		if (return_code == 1) {
			close(*socket_descriptor);
		}

		break;
	}

	// Return the valid address info
	return valid_address;
}

void cleanup(int descriptor, void *buffer) {
	close(descriptor);
	free(buffer);
}

int accept_communication(int socket_descriptor, struct Arguments *args) {
	// Data type big enough to hold both an sockaddr_in and sockaddr_in6 structure
	// The ai_addr structure contained in the addrinfo struct can point to either
	// an IPv4 sockaddr_in or an IPv6 sockaddr_in6 struct. Sometimes, we don't
	// know which will be returned from or which we have to pass to a function, so
	// the sockaddr_storage struct is large enough to hold either (you can then
	// cast as necessary)
	struct sockaddr_storage their_addr;

	// Data type to store the size of an sockaddr_storage object
	socklen_t sin_size = sizeof their_addr;

	// Start accepting connections on this address and machine.
	// This call will block until a client connects to the
	// socket_descriptor, and then returns a new file descriptor
	// just for communicating with this specific client. All
	// communication will then go through that socket. The server
	// could then just fork off a child to handle the client
	// and start accepting new clients on the socket_descriptor
	// clang-format off
	int connection = accept(
		socket_descriptor,
		(struct sockaddr *)&their_addr,
		&sin_size
	);
	// clang-format on

	if (connection == -1) {
		throw("Error accepting!");
	}

	// Make sure the buffer size is big enough for our messages
	adjust_socket_buffer_size(connection, args->size);

	return connection;
}

void communicate(int descriptor, struct Arguments *args) {
	struct Benchmarks bench;
	void *buffer;
	int message;

	setup_benchmarks(&bench);
	buffer = malloc(args->size);

	for (message = 0; message < args->count; ++message) {
		bench.single_start = now();

		// Send to the client
		if (send(descriptor, buffer, args->size, 0) == -1) {
			throw("Error sending from server");
		}

		// Read from client
		if (recv(descriptor, buffer, args->size, 0) == -1) {
			throw("Error receving from server");
		}

		benchmark(&bench);
	}

	evaluate(&bench, args);
	cleanup(descriptor, buffer);
}

int create_socket() {
	// Sockets are returned by the OS as standard file descriptors.
	// The first socket will be for the server's main connection port.
	// For every client that connects to that port (at that socket), we
	// get a new file descriptor just for communicating with precisely
	// that client.
	int socket_descriptor;

	// Address info structs are basic (relatively large) structures
	// containing various pieces of information about a host's address,
	// such as:
	// 1. ai_flags: A set of flags. If we set this to AI_PASSIVE, we can
	//              pass NULL to the later call to getaddrinfo for it to
	//              return the address info of the *local* machine (0.0.0.0)
	// 2. ai_family: The address family, either AF_INET, AF_INET6 or AF_UNSPEC
	//               (the latter makes this struct usable for IPv4 and IPv6)
	//               note that AF stands for Address Family.
	// 3. ai_socktype: The type of socket, either (TCP) SOCK_STREAM with
	//                 connection or (UDP) SOCK_DGRAM for connectionless
	//                 datagrams.
	// 4. ai_protocol: If you want to specify a certain protocol for the socket
	//                 type, i.e. TCP or UDP. By passing 0, the OS will choose
	//                 the most appropriate protocol for the socket type (STREAM
	//                 => TCP, DGRAM = UDP)
	// 5. ai_addrlen: The length of the address. We'll usually not modify this
	//                ourselves, but make use of it after it is populated via
	//                getaddrinfo.
	// 6. ai_addr: The Internet address. This is yet another struct, which
	//             basically contains the IP address and TCP/UDP port.
	// 7. ai_canonname: Canonical hostname.
	// 8. ai_next: This struct is actually a node in a linked list. getaddrinfo
	//             will sometimes return more than one address (e.g. one for IPv4
	//             one for IPv6)
	struct addrinfo hints, *server_info, *valid_address;

	// For system call return values
	int return_code;

	// Fill the hints with zeros first
	memset(&hints, 0, sizeof hints);
	// We set to AF_UNSPEC so that we can work
	// with either IPv6 or IPv4
	hints.ai_family = AF_UNSPEC;
	// Stream socket (TCP) as opposed to datagram sockets (UDP)
	hints.ai_socktype = SOCK_STREAM;
	// By setting this flag to AI_PASSIVE we can pass NULL for the hostname
	// in getaddrinfo so that the current machine hostname is implied
	//  hints.ai_flags = AI_PASSIVE;

	// This function will return address information for the given:
	// 1. Hostname or IP address (as string in digits-and-dots notation).
	// 2. The port of the address.
	// 3. The struct of hints we supply for the address.
	// 4. The addrinfo struct the function should populate with addresses
	//    (remember that addrinfo is a linked list)
	return_code = getaddrinfo(HOST, PORT, &hints, &server_info);

	if (return_code != 0) {
		fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(return_code));
		exit(EXIT_FAILURE);
	}

	valid_address = get_address(server_info, &socket_descriptor);

	// Don't need this anymore
	freeaddrinfo(server_info);

	// If we didn't actually find a valid address
	if (valid_address == NULL) {
		throw("Error finding valid address!");
	}

	// Now that we have a socket and an address bound to it, we
	// need to tell the OS that we wish to start listening for
	// incoming connections on this port.
	// Allow up to ten connections to queue up until the server
	// accepts them (the OS limit is often between 10 and 20)
	return_code = listen(socket_descriptor, 10);

	if (return_code == 1) {
		throw("Error listening on given socket!");
	}

	return socket_descriptor;
}

int main(int argc, char *argv[]) {
	// File-descriptor to the server socket
	int socket_descriptor;

	// File-descriptor for the socket over which
	// client-communication will take place
	int connection;

	// Command line arguments
	struct Arguments args;
	parse_arguments(&args, argc, argv);

	socket_descriptor = create_socket();
	connection = accept_communication(socket_descriptor, &args);

	// Don't need the main server descriptor anymore at this
	// point because we'll only communicate to the one client
	// for this benchmark.
	close(socket_descriptor);

	communicate(connection, &args);

	return EXIT_SUCCESS;
}
