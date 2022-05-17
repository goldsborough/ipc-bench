#include <string.h>

#include "common/arguments.h"
#include "common/utility.h"
#include "shm-sync-common.h"

void cleanup(struct Sync* sync) {
    // Detach the shared memory from this process' address space.
    // If this is the last process using this shared memory, it is removed.
    shmdt(sync->shared_memory);

    /*
        Deallocate manually for security. We pass:
            1. The shared memory ID returned by shmget.
            2. The IPC_RMID flag to schedule removal/deallocation
                 of the shared memory.
            3. NULL to the last struct parameter, as it is not relevant
                 for deletion (it is populated with certain fields for other
                 calls, notably IPC_STAT, where you would pass a struct shmid_ds*).
    */
    shmctl(sync->segment_id, IPC_RMID, NULL);
    if (sync->mutex_created) {
        destroy_sync(sync);
    }
}


void init_sync(struct Sync* sync) {
    // These structures are used to initialize mutexes
    // and condition variables. We will use them to set
    // the PTHREAD_PROCESS_SHARED attribute, which enables
    // more than one process (and any thread in any of those
    // processes) to access the mutex and condition variable.
    pthread_mutexattr_t mutex_attributes;
    pthread_condattr_t condition_attributes;

    // These methods initialize the attribute structures
    // with default values so that we must only change
    // the one we are interested in.
    if (pthread_mutexattr_init(&mutex_attributes) != 0) {
        throw("Error initializing mutex attributes");
    }
    if (pthread_condattr_init(&condition_attributes) != 0) {
        throw("Error initializing condition variable attributes");
    }

    // Here we set the "process-shared"-attribute of the mutex
    // and condition variable to PTHREAD_PROCESS_SHARED. This
    // means multiple processes may access these objects. If
    // we wouldn't do this, the attribute would be PTHREAD_PROCESS
    // _PRIVATE, where only threads created by the process who
    // initialized the objects would be allowed to access them.
    // By passing PTHREAD_PROCESS_SHARED the objects may also live
    // longer than the process creating them.
    // clang-format off
    if (pthread_mutexattr_setpshared(
                &mutex_attributes, PTHREAD_PROCESS_SHARED) != 0) {
        throw("Error setting process-shared attribute for mutex");
    }

    if (pthread_condattr_setpshared(
                &condition_attributes, PTHREAD_PROCESS_SHARED) != 0) {
        throw("Error setting process-shared attribute for condition variable");
    }
    // clang-format on

    // Initialize the mutex and condition variable and pass the attributes
    if (pthread_mutex_init(&sync->mutex->mutex, &mutex_attributes) != 0) {
        throw("Error initializing mutex");
    }
    if (pthread_cond_init(&sync->mutex->condition, &condition_attributes) != 0) {
        throw("Error initializing condition variable");
    }

    // Destroy the attribute objects
    if (pthread_mutexattr_destroy(&mutex_attributes)) {
        throw("Error destroying mutex attributes");
    }
    if (pthread_condattr_destroy(&condition_attributes)) {
        throw("Error destroying condition variable attributes");
    }

    sync->mutex->count = 0;
}

void destroy_sync(struct Sync* sync) {
    if (pthread_mutex_destroy(&sync->mutex->mutex)) {
        throw("Error destroying mutex");
    }
    if (pthread_cond_destroy(&sync->mutex->condition)) {
        throw("Error destroying condition variable");
    }
}

void sync_wait(struct Mutex *sync) {
    // Lock the mutex
    if (pthread_mutex_lock(&sync->mutex) != 0) {
        throw("Error locking mutex");
    }

    // Move into waiting for the condition variable to be signalled.
    // For this, it is essential that the mutex first be locked (above)
    // to avoid data races on the condition variable (e.g. the signal
    // being sent before the waiting process has begun). In fact, behaviour
    // is undefined otherwise. Once the mutex has begun waiting, the mutex
    // is unlocked so that other threads may do something and eventually
    // signal the condition variable. At that point, this thread wakes up
    // and *re-acquires* the lock immediately. As such, when this method
    // returns the lock will be owned by the calling thread.
    while (sync->count == 0) {
        if (pthread_cond_wait(&sync->condition, &sync->mutex) != 0) {
            throw("Error waiting for condition variable");
        }
    }
    sync->count --;
    if (pthread_mutex_unlock(&sync->mutex) != 0) {
        throw("Error unlocking mutex");
    }

}

void sync_notify(struct Mutex* sync) {
    if (pthread_mutex_lock(&sync->mutex) != 0) {
        throw("Error locking mutex");
    }
    // Signals to a single thread waiting on the condition variable
    // to wake up, if any such thread exists. An alternative would be
    // to call pthread_cond_broadcast, in which case *all* waiting
    // threads would be woken up.
    if (sync->count == 0) {
        if (pthread_cond_signal(&sync->condition) != 0) {
            throw("Error signalling condition variable");
        }
    }
    sync->count++;
    if (pthread_mutex_unlock(&sync->mutex) != 0) {
        throw("Error unlocking mutex");
    }
}

#include <stdio.h>
#include <errno.h>
#include <unistd.h>

void create_sync(struct Sync *sync, struct Arguments* args) {

    memset(sync, 0, sizeof(struct Sync));

    // The identifier for the shared memory segment

    // Key for the memory segment
    key_t segment_key = generate_key("shm");

    // The size for the segment
    int size = args->size + sizeof(*(struct Sync*)0);

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
    sync->segment_id = shmget(segment_key, size, IPC_CREAT | IPC_EXCL | 0666);
    sync->mutex_created  = 1;
    if (EEXIST == errno) {
        printf("[%d] shm exists\n", getpid());
        sync->segment_id = shmget(segment_key, size, 0666);
        sync->mutex_created = 0;
    }
    else {
        printf("[%d] create shm\n", getpid());
    }

    if (sync->segment_id < 0) {
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
    sync->shared_memory = shmat(sync->segment_id, NULL, 0);

    if (sync->shared_memory < 0) {
        throw("Could not attach segment");
    }

    memset(sync->shared_memory, 0, args->size);

    sync->mutex = (struct Mutex *)(sync->shared_memory + args->size);
    if (sync->mutex_created) {
        init_sync(sync);
    }

}
