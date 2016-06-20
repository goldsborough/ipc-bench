#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "common/utility.h"
#include "tssx/buffer.h"
#include "tssx/shared_memory.h"

#define SHM_FLAGS IPC_CREAT | IPC_EXCL | 0666

int create_segment(int buffer_size) {
	int id;
	int total_size;

	total_size = 2 * (sizeof(Buffer) + buffer_size);

	// Generate a random key until it is not taken yet
	while ((id = shmget(rand(), total_size, SHM_FLAGS)) == -1) {
		if (errno != EEXIST) {
			throw("Error creating segment");
		}
	}

	return id;
}

void* attach_segment(int segment_id) {
	void* shared_memory;

	if ((shared_memory = shmat(segment_id, NULL, 0)) == (void*)-1) {
		throw("Error attaching shared memory segment to address space");
	}

	return shared_memory;
}

void detach_segment(void* shared_memory) {
	if (shmdt(shared_memory) == -1) {
		throw("Error detaching shared memory segment");
	}
}

void destroy_segment(int segment_id) {
	if (shmctl(segment_id, IPC_RMID, NULL) == -1) {
		throw("Error destroying shared memory segment");
	}
}

int segment_size(Buffer* buffer) {
	return sizeof(Buffer) + buffer->capacity;
}
