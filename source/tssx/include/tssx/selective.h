#ifndef SELECTIVE_H
#define SELECTIVE_H

#include "tssx/definitions.h"

/******************** DEFINITIONS ********************/

#define ERROR -1

struct StringSet;
struct sockaddr;
struct sockaddr_un;
struct sockaddr_storage;

extern struct StringSet selective_set;

/******************** INTERFACE ********************/

int check_tssx_usage(int fd, Side side);

/******************** PRIVATE ********************/

int _in_selective_set(struct sockaddr_storage* address, size_t length);
int _is_domain_and_stream_socket(int fd, struct sockaddr_storage* address);
int _is_domain_socket(struct sockaddr_storage* address);
int _is_stream_socket(int fd);

void _initialize_selective_set();

const char* _fetch_tssx_variable();
void _parse_tssx_variable(const char* variable);

int _get_socket_option(int fd, int option_name);
int _get_socket_address(int fd, struct sockaddr_storage* address, Side side);

void _insert_null_terminator(struct sockaddr_un* address, size_t length);

#endif /* SELECTIVE_H */
