#define _XOPEN_SOURCE 500

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>

#include "common/utility.h"
#include "tssx/buffer.h"
#include "tssx/shared-memory.h"

#define SHM_FLAGS IPC_CREAT | IPC_EXCL | 0666

int create_segment(int total_size) {
	int id;

	// Generate a random key until it is not taken yet
	while ((id = shmget(rand(), total_size, SHM_FLAGS)) == -1) {
		if (errno != EEXIST && errno != EINVAL && errno != EACCES) {
			throw("Error creating segment");
		}
		// else just keep generating new keys (this one was taken)
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
