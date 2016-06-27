#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "tssx/selective.h"
#include "tssx/string-set.h"

StringSet selective_set = SS_INITIALIZER;

int check_use_tssx(int socket_fd) {
	int return_code;

	if ((return_code = is_domain_socket(socket_fd)) != 1) {
		// Either error or false (the socket is not a domain socket)
		return return_code;
	}

	if (!ss_is_initialized(&selective_set)) {
		initialize_selective_set();
	}

	// Empty means all sockets should use TSSX
	return ss_is_empty(&selective_set) || in_selective_set(socket_fd);
}

void initialize_selective_set() {
	const char* variable;
	if ((variable = fetch_tssx_variable()) != NULL) {
		parse_tssx_variable(variable);
	}
}

const char* fetch_tssx_variable() {
	return getenv("USE_TSSX");
}

void parse_tssx_variable(const char* variable) {
	char* buffer = (char*)malloc(strlen(variable));
	strcpy(buffer, variable);

	const char* path;
	for (path = strtok(buffer, " "); path; path = strtok(NULL, " ")) {
		assert(ss_insert(&selective_set, path));
	}

	free(buffer);
}

int in_selective_set(int socket_fd) {
	struct sockaddr_un address;
	socklen_t length = sizeof address;

	if (getsockname(socket_fd, (struct sockaddr*)&address, &length) == -1) {
		return ERROR;
	}

	return ss_contains(&selective_set, address.sun_path);
}

int is_domain_socket(int socket_fd) {
	struct sockaddr address;
	socklen_t length = sizeof address;

	if (getsockname(socket_fd, &address, &length) == -1) {
		return ERROR;
	}

	// AF_LOCAL is an alias for AF_UNIX (i.e. they have the same value)
	return address.sa_family == AF_LOCAL;
}

int get_socket_name(int socket_fd, char* destination) {
	struct sockaddr_un address;
	socklen_t length = sizeof address;

	if (getsockname(socket_fd, (struct sockaddr*)&address, &length) == -1) {
		return ERROR;
	}

	strcpy(destination, address.sun_path);

	return 0;
}
