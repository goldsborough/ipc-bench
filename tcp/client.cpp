#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

auto main() -> int {

  const int port = 6969;

  // Open a new socket and retrieve an entry in the file descriptor table
  // AF_INET:     internet domain (as opposed to unix file domain)
  // SOCK_STREAM: stream-oriented I/O
  // 0:           the protocol to use for the I/O; 0 means default for type
  //              (i.e. TCP for stream- and UDP for datagram-orientation)
  int socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_descriptor < 0) {
    throw std::runtime_error("Could not open socket!");
  }

  // hostent is a structure holding information for a host. It contains:
  // char*  h_name: The name of the host.
  // char** h_aliases: An array of alias strings for the host.
  // int    h_addrtype: The type of address, usually AF_INET.
  // int    h_length: The length of the address in bytes.
  // char** h_addr_list: A list of networ addresses for the named host.
  // #define h_addr h_addr_list[0]: Macro for backwards compatibility.

  // gethostbyname returns such a hostent* for a given hostname, or a
  // null-pointer if no such host exists
  hostent *server = gethostbyname("localhost");

  if (!server) {
    throw std::runtime_error("Could not find host!");
  }

  // sockaddr_in is a structure to model an internet address, it contains:
  // short sin_family: the connection domain (AF_INET)
  // u_short sin_port: the port number of the connection
  // sin_addr: another structure, containing only one field: the IP address of
  //           the host. For a server, this is always the current machine, which
  //           can be accessed via the macro INADDR_ANY
  // char sin_zero [8]: padding (I suppose)
  sockaddr_in server_address;

	// Set the address to be an internet address
  server_address.sin_family = AF_INET;

 	// htons stands for host-to-network-short, and converts the *short* from the
  // host's (i.e. this machine's) byte order (e.g. little-endian on most intel
  // machines) to network-byte-order, which is always big-endian.
	server_address.sin_port = htons(port);

	// Copy the address (in bytes) to the server address
  std::copy(server->h_addr,
						server->h_addr + server->h_length,
            reinterpret_cast<char*>(&server_address.sin_addr.s_addr));

	// Connects to the socket (server)
	auto return_code = connect(socket_descriptor,
														 reinterpret_cast<sockaddr*>(&server_address),
														 sizeof(server_address));

	if (return_code < 0) {
		throw std::runtime_error("Could not connect to socket!");
	}

	std::string input;

	std::cout << "Please enter a message: ";
	std::cin >> input;

	// Write data to the socket
	return_code = write(socket_descriptor, input.c_str(), input.length());

	if (return_code < 0) {
		throw std::runtime_error("Error while writing to socket!");
	}

	char buffer [256];
	std::fill(buffer, buffer + sizeof(buffer), 0);

	// Read data from the socket
	return_code = read(socket_descriptor, buffer, sizeof(buffer));

	if (return_code < 0) {
		throw std::runtime_error("Could not read from socket!");
	}

	std::cout << std::endl << buffer << std::flush;

	// Must close the socket and give it back to the process/OS
	close(socket_descriptor);
}
