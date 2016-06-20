#include <stddef.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "common/utility.h"
#include "tssx/buffer.h"
#include "tssx/shared_memory.h"

int create_segment(int buffer_size) {
	int key;
	int id;
	int total_size;

	total_size = 2 * (sizeof(Buffer) + buffer_size);
	key = generate_key(__FILE__);

	if ((id = shmget(key, total_size, IPC_CREAT | 0666)) == -1) {
		throw("Error getting segment");
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
