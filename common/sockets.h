#include <sys/socket.h>
#include <sys/types.h>

#define BUFFER_SIZE 64000

void adjust_socket_individual_buffer_size(int socket_descriptor, int which);

void adjust_socket_buffer_size(int socket_descriptor);

void adjust_socket_blocking_timeout(int socket_descriptor,
																		int seconds,
																		int microseconds);

int set_socket_flag(int socket_descriptor, int flag);

int receive(int connection, void* buffer, int size, int busy_waiting);
