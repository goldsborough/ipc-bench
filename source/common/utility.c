#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>

#include "utility.h"

void throw(const char* message) {
	perror(message);
	exit(EXIT_FAILURE);
}

int generate_key(const char* path) {
	// Generate a random key from the given file path
	// (inode etc.) plus the arbitrary character
	return ftok(path, 'X');
}
