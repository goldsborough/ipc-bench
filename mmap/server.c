#include <fcntl.h>
#include <sys/mman.h>

#include "common.h"

void make_space(int file_descriptor, int bytes) {
  lseek(file_descriptor, bytes + 1, SEEK_SET);
  write(file_descriptor, "", 1);
  lseek(file_descriptor, 0, SEEK_SET);
}

int get_file_descriptor(int bytes) {
  // Open a new file descriptor, creating the file if it does not exist
  // 0666 = read + write access for user, group and world
  int file_descriptor = open("/tmp/mmap", O_RDWR | O_CREAT, 0666);

  if (file_descriptor < 0) {
    throw("Error opening file!\n");
  }

  // Ensure that the file will hold enough space
  make_space(file_descriptor, bytes);

  return file_descriptor;
}

void write_data(char* file_memory, int bytes) {
	int start;

	while (*((char*)file_memory) != '1') usleep(1);

	start = now();
  memset(file_memory, '*', bytes);
	benchmark(start);

	*((char*)file_memory) = '2';
}

int main(int argc, const char *argv[]) {
  // The memory region to which the file will be mapped
  void *file_memory;

  // The number of bytes to write
  int bytes = get_bytes(argc, argv);

  // The file descriptor of the file we will
  // map into our process's memory
  int file_descriptor = get_file_descriptor(bytes);

  /*
		Arguments:
		1: Where to map the file to in the process' address
       space; NULL = let the OS find a region
		2: The size of the file in bytes
		3: The access rights, a bitwise OR of: PROT_READ, PROT_WRITE and PROT_EXEC
		4: Flags for the mapped memory, a bitwise OR of:
		   * MAP_FIXED: don't take the first argument as a hint, but really map
         the file there; you must ensure page-alignment,
       * MAP_PRIVATE: make a copy of the file if a write
			   occurs, i.e. the actual file will never change, only the *local* copy,
			 * MAP_SHARED: flush modified data to the underlying
         file immediately, instead of buffering; necessary for IPC!.
         Without MAP_SHARED, writes may be buffered by the OS.
			 * MAP_FILE: the default (zero) flag.
		5: The file descriptor to the opened file.
		6: The offset from the beginnning in the file, starting from which to map.
  */
	// clang-format off
  file_memory = mmap(
		NULL,
		bytes,
		PROT_READ | PROT_WRITE,
		MAP_SHARED,
		file_descriptor,
	  0
	 );
  // clang-format on

	if (file_memory < 0) {
    throw("Error mapping file!");
	}

	/*
		If you do not specify MAP_SHARED, but just MAP_FILE (the default),
		writes may be buffered by the OS. You can then flush the memory manually
		by calling msync(memory_address, memory_length, flags), where flags is the
		combination of:
		* MS_SYNC: Flush immediately, blocking until it's done.
		* MS_ASYNC: Schedule a flush, but the buffer may not necessarily have been
		  flushed before the call to msync returns.
		* MS_INVALIDATE: Invalidate all other process' cached data, so the change
		  will be visible to them.
		Usually one would use: MS_SYNC | MS_INVALIDATE, so:
		msync(file_memory, FILE_SIZE, MS_SYNC | MS_INVALIDATE);
	*/


  // Don't need the file descriptor anymore at this point
  if (close(file_descriptor) < 0) {
    throw("Error closing file!");
  }

	write_data(file_memory, bytes);

  // Unmap the file from the process memory
	// Actually unncessary because the OS will do
	// this automatically when the process terminates
  if (munmap(file_memory, bytes) < 0) {
    throw("Error unmapping file!");
	}

  return EXIT_SUCCESS;
}
