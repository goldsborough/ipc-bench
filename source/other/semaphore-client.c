#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>

union semaphore_union {
	int value;
	struct semid_ds* buf;
	// Array of semaphore counts for the semaphores
	unsigned short int* array;
	struct seminfo* __buf;
};

int semaphore_allocate(int fd) {
	// Allocate a set of semaphores, containing
	// only one semaphore (1), with permissions 0666
	return semget(key, 1, 0666);
}

int semaphore_deallocate(int fd) {
	// Dummy argument
	union semaphore_union _;
	return semctl(key, 1, IPC_RMID, _);
}

int semaphore_initialize(int semaphore_id) {
	union semaphore_union initializer;
	unsigned short values[1] = {1};
	initializer.array = values;
	return semctl(semaphore_id, 0, SETALL, intializer);
}

int semaphore_operation(int semaphore_id, int action) {
	// Array of operations for each semaphore
	struct sembuf operations[1];
	// Use the first (and only) semaphore in the set
	operations[0].sem_num = 0;
	// Modify the value somehow
	operations[0].sem_op = action;
	// When the process exits, have the OS undo
	// the value change of the semphore
	// Can also set IPC_NOWAIT, so that the semaphore
	// will never block, but fail the wait operation
	// when the semaphore value is 0
	operations[0].sem_flg = SEM_UNDO;

	return semop(semaphore_id, operations, sizeof sembuf);
}

int semaphore_wait(int semaphore_id) {
	// Decrement the value of the semaphore by one
	return semaphore_operation(semaphore_id, -1);
}

int semaphore_post() {
	// Increment the value of the semaphore by one
	return semaphore_operation(semaphore_id, +1);
}

int main(int argc, const char* argv[]) {
	int segment_id;
	int semaphore_id;
	char* shared_memory;

	// Pick a key for the semaphore set
	const int semaphore_key = 6969;

	if (argc != 2) {
		printf("Usage: client segment-key\n");
		exit(1);
	}

	segment_id = shmget(atoi(argv[1]), getpagesize(), 0666);

	if (segment_id < 0) {
		perror("Could not get segment");
		exit(1);
	}

	semaphore_id = semaphore_allocate(key);

	if (semaphore_id < 0) {
		perror("Error allocating semaphore!");
		exit(1);
	}

	if (semaphore_initialize(semaphore_id) < 0) {
		perror("Error intiializing semaphore!");
		exit(1);
	}

	shared_memory = shmat(segment_id, NULL, 0);

	if (shared_memory < (char*)0) {
		perror("Could not attach segment");
		exit(1);
	}


	semaphore_wait(semaphore_id);
	printf("%s\n", shared_memory);
	semaphore_post(semaphore_id);

	*shared_memory = '*';

	shmdt(shared_memory);

	semaphore_deallocate(key);

	return 0;
}
