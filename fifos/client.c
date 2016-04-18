#include <sys/stat.h>

#include "common.h"

void read_data(char *buffer, int bytes, FILE *stream) {
	if (fread(buffer, bytes, 1, stream) < 1) {
		printf("Error reading data!");
		exit(EXIT_FAILURE);
	}
}

void cleanup(char* buffer, FILE* stream) {
	free(buffer);
	fclose(stream);
	remove("/tmp/fifo");
}

int main(int argc, const char *argv[]) {
  // The number of bytes to send
  int bytes;
  // The buffer of data we allocate to send
  char *buffer;
	// The file pointer we will associate with the FIFO
	FILE* stream;

  // For benchmarking
  double start;

  bytes = get_bytes(argc, argv);

	buffer = (char *)malloc(bytes);

	if (buffer == NULL) {
		throw("Error allocating memory!\n");
	}

	// Because a FIFO is really just a file, we can
	// open a normal FILE* stream to it (in read mode)
	stream = fopen("/tmp/fifo", "r");

	// Start benchmarking
  start = now();

	// One pass read
	read_data(buffer, bytes, stream);

	// Stop benchmarking
  benchmark(start);

	cleanup(buffer, stream);

  return 0;
}
