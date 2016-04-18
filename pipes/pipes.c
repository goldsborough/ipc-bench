#include "common.h"

void read_data(char *buffer, int bytes, int file_descriptors[2]) {
  FILE *stream;
	int start;

  // Don't need the write end for the child
  close(file_descriptors[1]);

  // Open a new FILE stream in read mode from the file descriptor
  stream = fdopen(file_descriptors[0], "r");

	start = now();

  // Read the data (expect 1 object read)
  if (fread(buffer, bytes, 1, stream) < 1) {
		printf("Error reading data!\n");
		exit(EXIT_FAILURE);
	}

	benchmark(start);
	printf("(client)\n");

  // Now close the write end too
  close(file_descriptors[1]);
}

void write_data(char *buffer, int bytes, int file_descriptors[2]) {
  FILE *stream;
	int start;

  // Don't need the read end for the parent
  close(file_descriptors[0]);

  // Open a new FILE stream in write mode from the file descriptor
  stream = fdopen(file_descriptors[1], "w");

	start = now();

  // Write data
  if (fwrite(buffer, bytes, 1, stream) < 1) {
		printf("Erorr writing data!\n");
		exit(EXIT_FAILURE);
	}

	// Send immediately
  fflush(stream);

	benchmark(start);
	printf("(server)\n");

  // Now close the write end too
  close(file_descriptors[1]);
}

int main(int argc, const char *argv[]) {
  // The call to pipe will return two file descriptors
  // for the read and write end of the pipe, respectively
  int file_descriptors[2];
  // For the process ID of the spawned child process
  pid_t pid;
  // The number of bytes to send
  int bytes;
  // The buffer of data we allocate to send
  char *buffer;

  if (argc > 2) {
    printf("Usage: pipes [number of bytes to send]\n");
    exit(1);
  }

	else if (argc == 2) {
		bytes = atoi(argv[1]);
	}

	else {
		bytes = getpagesize();
	}

  buffer = (char *)malloc(bytes);

	if (buffer == NULL) {
		throw("Error allocating memory!\n");
	}

  // The call that creates a new pipe object and places two
  // valid file descriptors in the array we pass it. The first
  // entry at [0] is the read end (from which you read) and
  // second entry at [1] is the write end (to which you write)
  // Note that these file descriptors will only be visible to
  // the current process and any children it spawns. This is
  // mainly what distinguishes pipes from FIFOs (named pipes)
  if (pipe(file_descriptors) < 0) {
		free(buffer);
    throw("Error opening pipe!\n");
  }

  // Fork a child process
  if ((pid = fork()) < 0) {
		free(buffer);
    throw("Error forking process!\n");
  }

  // fork() returns 0 for the child process
  if (pid == (pid_t)0) {
    read_data(buffer, bytes, file_descriptors);
  }

  else {
    write_data(buffer, bytes, file_descriptors);

  }

  free(buffer);

  return 0;
}
