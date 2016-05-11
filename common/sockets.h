#include <sys/socket.h>
#include <sys/types.h>

#define BUFFER_SIZE 64000

void adjust_socket_individual_buffer_size(int socket_descriptor, int which);

void adjust_socket_buffer_size(int socket_descriptor);
