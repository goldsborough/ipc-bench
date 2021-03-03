#ifndef SHM_SYNC_COMMON_H
#define SHM_SYNC_COMMON_H

#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>

struct Arguments;

struct Mutex {
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    int count;
};

struct Sync {
    int segment_id;
    void *shared_memory;
    struct Mutex *mutex;
    int mutex_created;
};

void cleanup(struct Sync* sync);

void init_sync(struct Sync* sync);

void destroy_sync(struct Sync* sync);

void sync_wait(struct Mutex* sync);

void sync_notify(struct Mutex* sync);

void create_sync(struct Sync* sync, struct Arguments* args);

#endif /* SHM_SYNC_COMMON_H */
