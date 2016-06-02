#include <sys/socket.h>
#include <sys/types.h>

#define BUFFER_SIZE 64000

#define SEND 0
#define RECEIVE 1

struct timeval;
typedef struct timeval timeval;

int get_socket_buffer_size(int socket_fd, int which);
timeval get_socket_timeout(int socket_fd, int which);
double get_socket_timeout_seconds(int socket_fd, int which);

void set_socket_buffer_size(int socket_fd, int which);
void set_socket_both_buffer_sizes(int socket_fd);

void set_socket_timeout(int socket_fd, timeval* timeout, int which);
void set_socket_both_timeouts(int socket_fd, int seconds, int microseconds);

int set_io_flag(int socket_fd, int flag);

int receive(int connection, void* buffer, int size, int busy_waiting);
