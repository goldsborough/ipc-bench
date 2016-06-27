#ifndef SELECTIVE_H
#define SELECTIVE_H

#define ERROR -1
#define

struct StringSet;

extern struct StringSet selective_set;

int check_use_tssx(int socket_fd);

int in_selective_set(int socket_fd);
int is_domain_socket(int socket_fd);
int get_socket_name(int socket_fd, char* destination);

void initialize_selective_set();

const char* fetch_tssx_variable();
void parse_tssx_variable(const char* variable);

#endif /* SELECTIVE_H */
