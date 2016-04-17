#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

void throw(const char *message) {
  perror(message);
  exit(EXIT_FAILURE);
}

double now() {
	return ((double)clock()) / CLOCKS_PER_SEC * 1e6;
}

void benchmark(int start) {
	printf("%f\n", now() - start);
}

int get_bytes(int argc, const char* argv[]) {
	if (argc > 2) {
    printf("Usage: fifos [number of bytes to send]\n");
    exit(1);
  }

  else if (argc == 2) {
    return atoi(argv[1]);
  }

  else {
    return getpagesize();
  }
}
