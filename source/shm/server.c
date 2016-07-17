#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <unistd.h>

#include "common/common.h"

void cleanup(int segment_id, char* shared_memory) {
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
}

void shm_wait(atomic_char* guard) {
	while (atomic_load(guard) != 's')
		;
}

void shm_notify(atomic_char* guard) {
	atomic_store(guard, 'c');
}

void communicate(char* shared_memory, struct Arguments* args) {
	struct Benchmarks bench;
	int message;
	void* buffer = malloc(args->size);
	atomic_char* guard = (atomic_char*)shared_memory;

	// Wait for signal from client
	shm_wait(guard);
	setup_benchmarks(&bench);

	for (message = 0; message < args->count; ++message) {
		bench.single_start = now();

		// Write
		memset(shared_memory + 1, '*', args->size);

		shm_notify(guard);
		shm_wait(guard);

		// Read
		memcpy(buffer, shared_memory + 1, args->size);

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
	char* shared_memory;

	// Key for the memory segment
	key_t segment_key;

	// Fetch command-line arguments
	struct Arguments args;
	parse_arguments(&args, argc, argv);

	segment_key = generate_key("shm");

	/*
		The call that actually allocates the shared memory segment.
		Arguments:
			1. The shared memory key. This must be unique across the OS.
			2. The number of bytes to allocate. This will be rounded up to the OS'
				 pages size for alignment purposes.
			3. The creation flags and permission bits, where:
				 - IPC_CREAT means that a new segment is to be created
				 - IPC_EXCL means that the call will fail if
					 the segment-key is already taken (removed)
				 - 0666 means read + write permission for user, group and world.
		When the shared memory key already exists, this call will fail. To see
		which keys are currently in use, and to remove a certain segment, you
		can use the following shell commands:
			- Use `ipcs -m` to show shared memory segments and their IDs
			- Use `ipcrm -m <segment_id>` to remove/deallocate a shared memory segment
	*/
	segment_id = shmget(segment_key, 1 + args.size, IPC_CREAT | 0666);

	if (segment_id < 0) {
		throw("Error allocating segment");
	}

	/*
		Once the shared memory segment has been created, it must be
		attached to the address space of each process that wishes to
		use it. For this, we pass:
			1. The segment ID returned by shmget
			2. A pointer at which to attach the shared memory segment. This
				 address must be page-aligned. If you simply pass NULL, the OS
				 will find a suitable region to attach the segment.
			3. Flags, such as:
				 - SHM_RND: round the second argument (the address at which to
					 attach) down to a multiple of the page size. If you don't
					 pass this flag but specify a non-null address as second argument
					 you must ensure page-alignment yourself.
				 - SHM_RDONLY: attach for reading only (independent of access bits)
		shmat will return a pointer to the address space at which it attached the
		shared memory. Children processes created with fork() inherit this segment.
	*/
	shared_memory = (char*)shmat(segment_id, NULL, 0);

	if (shared_memory == (char*)-1) {
		throw("Error attaching segment");
	}

	communicate(shared_memory, &args);

	cleanup(segment_id, shared_memory);

	return EXIT_SUCCESS;
}
