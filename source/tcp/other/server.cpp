#include <cstdint>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

auto main() -> int {

  const char *host_name = "localhost";
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

  // sockaddr_in is a structure to model an internet address, it contains:
  // short sin_family: the connection domain (AF_INET)
  // u_short sin_port: the port number of the connection
  // sin_addr: another structure, containing only one field: the IP address of
  //           the host. For a server, this is always the current machine, which
  //           can be accessed via the macro INADDR_ANY
  // char sin_zero [8]: padding (I suppose)
  sockaddr_in server_address;

  // Configure the server's internet address
  server_address.sin_family = AF_INET;

	// INADRR_ANY means the machine's current IP
  server_address.sin_addr.s_addr = INADDR_ANY;

	// htons stands for host-to-network-short, and converts the *short* from the
  // host's (i.e. this machine's) byte order (e.g. little-endian on most intel
  // machines) to network-byte-order, which is always big-endian.
  server_address.sin_port = htons(port);

  // Bind the socket to the server address
  auto return_code =
      bind(socket_descriptor, reinterpret_cast<sockaddr *>(&server_address),
           sizeof(server_address));

  if (return_code < 0) {
    throw std::runtime_error("Could not bind socket to ");
  }

  // Configure the process to be able to listen on the socket
  // Allow up to five connections to wait while the server
  // processes another host (maximum allowed by most systems)
  listen(socket_descriptor, 5);

  sockaddr_in client_address;
  socklen_t client_address_length = sizeof(client_address);

  // Start accepting communication on the given socket,
  // blocking while waiting for any client to connect()
  // to the given socket (address). This function returns
  // a new socket file descriptor, on which the actual
  // communication then takes place
  auto communication_socket =
      accept(socket_descriptor, reinterpret_cast<sockaddr *>(&client_address),
             &client_address_length);

  if (communication_socket < 0) {
    throw std::runtime_error("Error while accepting socket connection!");
  }

  // A connection to a client has been established at this point

  std::uint8_t buffer[256];
  std::fill(buffer, buffer + sizeof(buffer), 0);

  // Read data from the socket, into the
  // buffer, at most sizeof(buffer) bytes
  return_code = read(communication_socket, buffer, sizeof(buffer));

  if (return_code < 0) {
    throw std::runtime_error("Error while reading from socket!");
  }

  std::cout << "Received message: " << buffer << std::endl;

  // Write a message to the socket
  return_code =
      write(communication_socket, "Message transmission successful!", 32);

  if (return_code < 0) {
    throw std::runtime_error("Error while writing acknowledgement message!");
  }

  // Have to release the file descriptors back to the process/OS
  close(communication_socket);
  close(socket_descriptor);
}
