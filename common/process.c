#include <assert.h>
#include <string.h>
#include <unistd.h>

#include "common/utility.h"

void start_process(char *argv[]) {
	// Will need to set the group id
	const pid_t parent_pid = getpid();
	const pid_t pid = fork();

	if (pid == 0) {
		// Set group id of the children so that we
		// can send around signals
		if (setpgid(pid, parent_pid) == -1) {
			throw("Could not set group id for child process");
		}
		// Replace the current process with the command
		// we want to execute (child or server)
		// First argument is the command to call,
		// second is an array of arguments, where the
		// command path has to be included as well
		// (that's why argv[0] first)
		if (execv(argv[0], argv) == -1) {
			throw("Error opening child process");
		}
	}
}

void copy_arguments(char *arguments[], int argc, char *argv[]) {
	int i;
	assert(argc < 8);
	for (i = 1; i < argc; ++i) {
		arguments[i] = argv[i];
	}

	arguments[argc] = NULL;
}

void start_server(char *name, int argc, char *argv[]) {
	char *arguments[8] = {name};
	copy_arguments(arguments, argc, argv);
	start_process(arguments);
}

void start_client(char *name, int argc, char *argv[]) {
	char *arguments[8] = {name};
	copy_arguments(arguments, argc, argv);
	start_process(arguments);
}

void start_children(char *prefix, int argc, char *argv[]) {
	char server_name[32] = "./";
	char client_name[32] = "./";

	strcat(server_name, prefix);
	strcat(client_name, prefix);

	strcat(server_name, "-server");
	strcat(client_name, "-client");

	start_server(server_name, argc, argv);
	start_client(client_name, argc, argv);
}
