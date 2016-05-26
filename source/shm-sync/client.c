#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common/common.h"
#include "shm-sync-common.h"

void cleanup(void* shared_memory) {
	// Detach the shared memory from this process' address space.
	// If this is the last process using this shared memory, it is removed.
	shmdt(shared_memory);
}

void communicate(void* shared_memory,
								 struct Arguments* args,
								 struct Sync* sync) {
	// Buffer into which to read data
	void* buffer = malloc(args->size);

	sync_notify(sync);

	printf("4\n");

	for (; args->count > 0; --args->count) {
		sync_wait(sync);

		printf("5\n");

		// Read from memory
		memcpy(buffer, shared_memory, args->size);
		// Write back
		memset(shared_memory, '2', args->size);

		printf("6\n");

		sync_notify(sync);

		printf("7\n");
	}

	free(buffer);
}

int main(int argc, char* argv[]) {
	// The identifier for the shared memory segment
	int segment_id;

	// The *actual* shared memory, that this and other
	// processes can read and write to as if it were
	// any other plain old memory
	void* shared_memory;

	// The synchronization object
	struct Sync* sync;

	// Fetch command-line arguments
	struct Arguments args;
	parse_arguments(&args, argc, argv);

	segment_id = create_segment(&args);
	shared_memory = attach_segment(segment_id, &args);
	sync = create_sync(shared_memory, &args);

	communicate(shared_memory, &args, sync);

	cleanup(shared_memory);

	return EXIT_SUCCESS;
}
