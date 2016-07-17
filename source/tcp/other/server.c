#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void sigchld_handler(int _) {
	// waitpid() might modify the errno, so we save it
	int saved_errno = errno;

	while (waitpid(-1, NULL, WNOHANG) > 0)
		;

	// and restore the errno
	errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa) {
	// AF_INET = Address Family INET
	if (sa->sa_family == AF_INET) {
		// Cast the raw address protocol part in sockaddr
		// to the more accessible sockaddr_in structure
		// so we can aaccess the s_addr field
		struct sockaddr_in *in = (struct sockaddr_in *)sa;
		return &(in->sin_addr);
	} else {
		struct sockaddr_in6 *in = (struct sockaddr_in6 *)sa;
		return &(in->sin6_addr);
	}
}

void print_address(struct addrinfo *address_info) {
	char *type;
	void *address;
	char ip[INET6_ADDRSTRLEN];

	// IPv4
	if (address_info->ai_family == AF_INET) {
		struct sockaddr_in *ipv4 = (struct sockaddr_in *)address_info->ai_addr;
		address = &(ipv4->sin_addr);
		type = "IPv4";
	} else {// IPv6
		struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)address_info->ai_addr;
		address = &(ipv6->sin6_addr);
		type = "IPv6";
	}

	inet_ntop(address_info->ai_family, address, ip, sizeof ip);
	fprintf(stderr, "%s: %s ... ", type, ip);
}

#define PORT "3490"
#define true 1

int main(int argc, const char *argv[]) {
	// Socket file descriptors
	int socket_fd, new_fd;
	// Host address information structs
	struct addrinfo hints, *server_info, *iterator;
	// Data type big enough to hold both an sockaddr_in and sockaddr_in6 structure
	struct sockaddr_storage their_addr;
	// Data type to store the size of an sockaddr_storage object
	socklen_t sin_size;
	// Will be necessary when calling setsockopt to free busy sockets
	int yes = 1;
	// To hold an ipv6 address (or smaller)
	char ip[INET6_ADDRSTRLEN];
	// For system call return values
	int return_code;

	// Fill with zeros first to clear all data
	memset(&hints, 0, sizeof hints);
	// Set the address family to internet as opposed to, e.g. unix files
	// AF_UNSPEC as opposed to AF_INET or AF_INET6
	hints.ai_family = AF_UNSPEC;
	// Stream socket (TCP) as opposed to datagram sockets (UDP)
	hints.ai_socktype = SOCK_STREAM;
	// By setting this flag to AI_PASSIVE we can pass NULL for the hostname
	// in getaddrinfo so that the current machine hostname is implied
	hints.ai_flags = AI_PASSIVE;

	// Fetch address info for our server machine
	if ((return_code = getaddrinfo(NULL, PORT, &hints, &server_info)) != 0) {
		fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(return_code));
		exit(1);
	}

	// Iterate through the addresses until we find one we can get a socket for
	for (iterator = server_info; iterator != NULL; iterator = iterator->ai_next) {
		print_address(iterator);

		// Try to get a socket file_descriptor for this address
		if ((socket_fd = socket(iterator->ai_family,
														iterator->ai_socktype,
														iterator->ai_protocol)) == -1) {
			perror("could not connect!");
			continue;
		}

		// If the socket is blocked, tell the OS to reclaim it
		if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) ==
				-1) {
			perror("could not reclaim socket!");
			exit(1);
		}

		// Try to bind the socket to this address
		if (bind(socket_fd, iterator->ai_addr, iterator->ai_addrlen) == 1) {
			close(socket_fd);
			perror("could not bind socket to address!");
		}

		break;
	}

	freeaddrinfo(server_info);

	// If we didn't actually find a valid address
	if (iterator == NULL) {
		perror("Could not find valid address!");
		exit(1);
	}

	if (listen(socket_fd, 10) == 1) {
		perror("Could not start listening on socket!");
		exit(1);
	}

	printf("Server: Wainting for connections ...");

	while (true) {
		sin_size = sizeof their_addr;
		// Start accepting connections (blocks)
		// This will return a new socket file descriptor through
		// which all communication with this specific client will go
		if ((new_fd =
						 accept(socket_fd, (struct sockaddr *)&their_addr, &sin_size)) ==
				-1) {
			perror("Error accepting!");
			exit(1);
		}

		// Translate the received connection from bytes to representation
		inet_ntop(their_addr.ss_family,
							get_in_addr((struct sockaddr *)&their_addr),
							ip,
							sizeof ip);
		printf("Server: Conneted to: %s\n", ip);

		// Fork the process at this point
		if (fork() == 0) {
			// Child process doesn't need the old file descriptor
			close(socket_fd);
			if (send(new_fd, "Hello, world!", 13, 0) == -1) {
				perror("Failed to send message to connected client!");
				exit(1);
			}

			close(new_fd);
			exit(0);
		}

		// Parent doesn't need this
		close(new_fd);
	}

	return 0;
}
