#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common/common.h"
#include "shm-sync-common.h"

void cleanup(int segment_id, void* shared_memory, struct Sync* sync) {
	// Detach the shared memory from this process' address space.
	// If this is the last process using this shared memory, it is removed.
	shmdt(shared_memory);

	/*
		Deallocate manually for security. We pass:
			1. The shared memory ID returned by shmget.
			2. The IPC_RMID flag to schedule removal/deallocation
				 of the shared memory.
			3. NULL to the last struct parameter, as it is not relevant
				 for deletion (it is populated with certain fields for other
				 calls, notably IPC_STAT, where you would pass a struct shmid_ds*).
	*/
	shmctl(segment_id, IPC_RMID, NULL);

	destroy_sync(sync);
}


void communicate(void* shared_memory,
								 struct Arguments* args,
								 struct Sync* sync) {
	struct Benchmarks bench;
	int message;
	void* buffer = malloc(args->size);

	// Wait for signal from client
	sync_wait(sync);
	setup_benchmarks(&bench);

	for (message = 0; message < args->count; ++message) {
		bench.single_start = now();

		printf("1\n");

		// Write into the memory
		memset(shared_memory, '1', args->size);

		sync_notify(sync);
		sync_wait(sync);

		printf("2\n");

		// Read
		memcpy(buffer, shared_memory, args->size);

		sync_notify(sync);

		printf("3\n");

		benchmark(&bench);
	}

	evaluate(&bench, args);
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

	cleanup(segment_id, shared_memory, sync);

	return EXIT_SUCCESS;
}
