#include <sys/stat.h>

#include "common.h"

void write_data(char* buffer, int bytes, FILE* stream) {
	int start = now();
	fwrite(buffer, bytes, 1, stream);
	benchmark(start);
}

void cleanup(char* buffer, FILE* stream) {
	free(buffer);
  fclose(stream);
}

int main(int argc, const char *argv[]) {
  // The number of bytes to send
  int bytes;
  // The buffer of data we allocate to send
  char *buffer;
  // The file pointer we will associate with the FIFO
  FILE *stream;

  bytes = get_bytes(argc, argv);

  buffer = (char *)malloc(bytes);

  if (buffer == NULL) {
    throw("Error allocating memory!\n");
  }

	// Create a FIFO object. Note that a FIFO is
	// really just a special file, which must be
	// opened by one process and to which then
	// both server and client can write using standard
	// c i/o functions. 0666 specifies read+write
	// access permissions for the user, group and world
  if (mkfifo("/tmp/fifo", 0666) < 0) {
		free(buffer);
    throw("Error opening FIFO!\n");
  }

  // Because a fifo is really just a file, we can
  // open a normal FILE* stream to it (in write mode)
	// Observation: this call blocks until the FIFO has
	// been opened in read mode by the client?
  stream = fopen("/tmp/fifo", "w");

	write_data(buffer, bytes, stream);

  cleanup(buffer, stream);

  return 0;
}
