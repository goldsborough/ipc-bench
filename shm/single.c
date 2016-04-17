#include <stdio.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
	// *Our* key for the shared memory segment
	const int segment_key = 7005;
	// The OS' ID for the shared memory segment
	int segment_id;
	// The *actual shared memory* buffer
	char* shared_memory;
	// Struct to hold information about shared memory
	struct shmid_ds stat_buffer;
	// The shared memory size we'd like
	const int shared_segment_size = getpagesize();

	// Allocate a new shared memory segment
	segment_id = shmget(
		segment_key,
		shared_segment_size,
		IPC_CREAT | IPC_EXCL | 0666
	);

	if (segment_id < 0) {
		perror("Error allocating segment!");
		exit(1);
	}

	// Now we attach the newly allocated
	// segment to our processe's address space
	shared_memory = shmat(segment_id, NULL, 0);

	if (shared_memory < 0) {
		perror("Error attaching segment!");
		// Deallocate
		shmctl(segment_id, IPC_RMID, NULL);
		exit(1);
	}

	printf("Attached shared memory at %p\n", shared_memory);

	// Retrieve information about the segment
	// Store into the stat_buffer
	shmctl(segment_id, IPC_STAT, &stat_buffer);

	printf("The shared memory is %d bytes long\n", stat_buffer.shm_segsz);

	// Write to the buffer!
	sprintf(shared_memory, "Hello, World!");

	// Detach the shared memory from its current space
	/*
	if (shmdt(shared_memory) < 0) {
		perror("Error detaching segment!");
		shmctl(segment_id, IPC_RMID, NULL);
		exit(1);
	}
	*/


	// Reattach/remap the shared memory to a different
	// address in virtual memory
	shared_memory = shmat(segment_id, NULL, SHM_RND);

	if (shared_memory < (char*)0) {
		perror("Error attaching segment!");
		// Deallocate
		shmctl(segment_id, IPC_RMID, NULL);
		exit(1);
	}

	printf("Reattached the shared memory segment at %p\n", shared_memory);

	printf("%s\n", shared_memory);

	if (shmdt(shared_memory) < 0) {
		perror("Error detaching segment!");
		shmctl(segment_id, IPC_RMID, NULL);
		exit(1);
	}

	// Deallocate
	shmctl(segment_id, IPC_RMID, NULL);

	return 0;
}
