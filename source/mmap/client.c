#include <fcntl.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "common/common.h"

int get_file_descriptor() {
	// Open a new file descriptor, creating the file if it does not exist
	// 0666 = read + write access for user, group and world
	int file_descriptor = open("/tmp/mmap", O_RDWR | O_CREAT, 0666);

	if (file_descriptor < 0) {
		throw("Error opening file!\n");
	}

	return file_descriptor;
}

void mmap_wait(atomic_char* guard) {
	while (atomic_load(guard) != 'c')
		;
}

void mmap_notify(atomic_char* guard) {
	atomic_store(guard, 's');
}

void communicate(char* file_memory, struct Arguments* args) {
	// Buffer into which to read data
	void* buffer = malloc(args->size);
	atomic_char* guard = (atomic_char*)file_memory;

	mmap_notify(guard);

	for (; args->count > 0; --args->count) {
		mmap_wait(guard);

		memcpy(buffer, file_memory, args->size);
		memset(file_memory, '*', args->size);

		mmap_notify(guard);
	}

	free(buffer);
}


int main(int argc, char* argv[]) {
	// The memory region to which the file will be mapped
	void* file_memory;
	// The file descriptor of the file we will
	// map into our process's memory
	int file_descriptor;
	// Fetch command-line arguments
	struct Arguments args;

	parse_arguments(&args, argc, argv);
	file_descriptor = get_file_descriptor();

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
		args.size,
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

	communicate(file_memory, &args);

	// Unmap the file from the process memory
	// Actually unncessary because the OS will do
	// this automatically when the process terminates
	if (munmap(file_memory, args.size) < 0) {
		throw("Error unmapping file!");
	}

	remove("/tmp/mmap");

	return EXIT_SUCCESS;
}
