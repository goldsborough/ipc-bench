#include <stdio.h>
#include <stdlib.h>

#include "utility.h"

void throw(const char *message) {
	perror(message);
	exit(EXIT_FAILURE);
}
