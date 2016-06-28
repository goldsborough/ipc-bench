#ifndef SELECTIVE_H
#define SELECTIVE_H

#include <stdbool.h>

#define ERROR -1

struct StringSet;
struct sockaddr;
extern struct StringSet selective_set;

int server_check_use_tssx(int socket_fd);
int client_check_use_tssx(int socket_fd, const struct sockaddr* address);

int in_selective_set(int socket_fd);
int is_domain_and_stream_socket(int socket_fd);
int is_domain_socket(int socket_fd);
int is_stream_socket(int socket_fd);
int get_socket_option(int socket_fd, int option_name);

void initialize_selective_set();

const char* fetch_tssx_variable();
void parse_tssx_variable(const char* variable);

#endif /* SELECTIVE_H */
