#ifndef SHM_SYNC_COMMON_H
#define SHM_SYNC_COMMON_H

#include <pthread.h>
#include <sys/shm.h>

struct Arguments;

struct Sync {
	pthread_mutex_t mutex;
	pthread_cond_t condition;
};

void init_sync(struct Sync* sync);

void destroy_sync(struct Sync* sync);

void sync_wait(struct Sync* sync);

void sync_notify(struct Sync* sync);


int create_segment(struct Arguments* args);

void* attach_segment(int segment_id, struct Arguments* args);

struct Sync* create_sync(void* shared_memory, struct Arguments* args);

#endif /* SHM_SYNC_COMMON_H */
