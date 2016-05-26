#include <sys/ipc.h>
#include <sys/shm.h>

#include "common/utility.h"
#include "shared_memory.h"
#incldue "buffer.h"

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
	shmdt(shared_memory);
}

void destroy_segment(int segment_id) {
	shmctl(segment_id, IPC_RMID, NULL);
}

int segment_size(Buffer* buffer) {
	return sizeof(Buffer) + buffer->capacity;
}
