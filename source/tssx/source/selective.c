#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#include "common/utility.h"
#include "tssx/selective.h"
#include "tssx/string-set.h"

StringSet selective_set = SS_INITIALIZER;

int check_tssx_usage(int fd, Side side) {
	struct sockaddr_storage address;
	size_t length;
	int return_code;

	length = _get_socket_address(fd, &address, side);

	if ((return_code = _is_domain_and_stream_socket(fd, &address)) != true) {
		// Either error (-1) or false (0) (the socket is not a domain socket)
		return return_code;
	}

	if (!ss_is_initialized(&selective_set)) {
		_initialize_selective_set();
	}

	// Empty means all sockets should use TSSX
	return _in_selective_set(&address, length);
}

void _initialize_selective_set() {
	const char* variable;
	if ((variable = _fetch_tssx_variable()) != NULL) {
		_parse_tssx_variable(variable);
	}

#ifdef DEBUG
	if (variable == NULL) {
		fprintf(stderr, "'USE_TSSX' not found, disabling TSSX by default ...\n");
	}
#endif
}

const char* _fetch_tssx_variable() {
	return getenv("USE_TSSX");
}

void _parse_tssx_variable(const char* variable) {
	char* buffer;

	// Not allowed to change the variable buffer
	buffer = malloc(strlen(variable));
	strcpy(buffer, variable);

	const char* path;
	for (path = strtok(buffer, " "); path; path = strtok(NULL, " ")) {
#ifdef DEBUG
		assert(ss_insert(&selective_set, path));
#else
		ss_insert(&selective_set, path);
#endif
	}

	free(buffer);
}

int _in_selective_set(struct sockaddr_storage* address, size_t length) {
	struct sockaddr_un* domain_address = (struct sockaddr_un*)address;

	// Seems this really is necessary
	_insert_null_terminator(domain_address, length);

#ifdef DEBUG
	if (ss_contains(&selective_set, domain_address->sun_path)) {
		fprintf(stderr, "Enabling TSSX for '%s' ...\n", domain_address->sun_path);
	} else {
		fprintf(stderr, "Disabling TSSX for '%s' ...\n", domain_address->sun_path);
	}
#endif

	return ss_contains(&selective_set, domain_address->sun_path);
}

int _is_domain_and_stream_socket(int fd, struct sockaddr_storage* address) {
	return _is_domain_socket(address) && _is_stream_socket(fd);
}

int _is_domain_socket(struct sockaddr_storage* address) {
	// AF_LOCAL is an alias for AF_UNIX (i.e. they have the same value)
	return address->ss_family == AF_LOCAL;
}

int _is_stream_socket(int fd) {
	int option = _get_socket_option(fd, SO_TYPE);
	return option == ERROR ? ERROR : (option == SOCK_STREAM);
}

int _get_socket_option(int fd, int option_name) {
	int return_code;
	socklen_t option;
	socklen_t option_length = sizeof option;

	// clang-format off
	return_code = getsockopt(
    fd,
    SOL_SOCKET,
    option_name,
    &option,
    &option_length
  );
	// clang-format on

	if (return_code == -1) {
		print_error("Error getting socket option");
		return ERROR;
	}

	return option;
}

int _get_socket_address(int fd, struct sockaddr_storage* address, Side side) {
	socklen_t length = sizeof *address;
	int return_code;

	if (side == SERVER) {
		return_code = getsockname(fd, (struct sockaddr*)address, &length);
	} else {
		return_code = getpeername(fd, (struct sockaddr*)address, &length);
	}

	if (return_code == ERROR) {
		fprintf(stderr, "Error getting socket address");
		return ERROR;
	}

	return length;
}

void _insert_null_terminator(struct sockaddr_un* address, size_t length) {
	length -= offsetof(struct sockaddr_un, sun_path);
	address->sun_path[length] = '\0';
}
