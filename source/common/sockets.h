#ifndef SOCKETS_H
#define SOCKETS_H

#include <stdbool.h>
#include <sys/socket.h>
#include <sys/types.h>

/******************** DEFINITIONS ********************/

#define BUFFER_SIZE 64000

typedef enum Direction { SEND, RECEIVE } Direction;

struct timeval;
typedef struct timeval timeval;

/******************** INTERFACE ********************/

int socket_buffer_size(int socket_fd, Direction direction);

void set_socket_buffer_size(int socket_fd, Direction direction);
void set_socket_both_buffer_sizes(int socket_fd);

timeval socket_timeout(int socket_fd, Direction direction);
double socket_timeout_seconds(int socket_fd, Direction direction);

void set_socket_timeout(int socket_fd, timeval* timeout, Direction direction);
void set_socket_both_timeouts(int socket_fd, int seconds, int microseconds);

int get_socket_flags(int socket_fd);
void set_socket_flags(int socket_fd, int flags);

int set_socket_non_blocking(int socket_fd);
int unset_socket_non_blocking(int socket_fd);

bool socket_is_non_blocking(int socket_fd);

int set_io_flag(int socket_fd, int flag);

int receive(int connection, void* buffer, int size, int busy_waiting);

#endif /* SOCKETS_H */
