#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "common/utility.h"

#define BUILD_PATH "/build/source\0"

char *find_build_path() {
	char *path = (char *)malloc(strlen(__FILE__) + strlen(BUILD_PATH));
	char *right;
	char *left;

	strcpy(path, __FILE__);
	right = path + strlen(__FILE__);
	left = right - strlen("ipc-bench");

	// Make the string-comparison expected O(N)
	// by only doing a string-comparison when
	// the first and last character match
	for (--right; left >= path; --left, --right) {
		if (*left == 'i' && *right == 'h') {
			if (strncmp("ipc-bench", left, 9) == 0) {
				break;
			}
		}
	}

	// ++ because right is on the 'h'
	strcpy(++right, BUILD_PATH);

	return path;
}


pid_t start_process(char *argv[]) {
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

	return pid;
}

void copy_arguments(char *arguments[], int argc, char *argv[]) {
	int i;
	assert(argc < 8);
	for (i = 1; i < argc; ++i) {
		arguments[i] = argv[i];
	}

	arguments[argc] = NULL;
}

pid_t start_child(char *name, int argc, char *argv[]) {
	char *arguments[8] = {name};
	copy_arguments(arguments, argc, argv);
	return start_process(arguments);
}

void start_children(char *prefix, int argc, char *argv[]) {
	char server_name[100];
	char client_name[100];

	char *build_path = find_build_path();

	// clang-format off
	sprintf(
		server_name,
		"%s/%s/%s-%s",
		build_path,
		prefix,
		prefix,
		"server"
	);

	sprintf(
		client_name,
		"%s/%s/%s-%s",
		build_path,
		prefix,
		prefix,
		"client"
	);
	// clang-format on

	pid_t c1_id = start_child(server_name, argc, argv);
	pid_t c2_id = start_child(client_name, argc, argv);

	waitpid(c1_id, NULL, WUNTRACED);
	waitpid(c2_id, NULL, WUNTRACED);

	free(build_path);
}
