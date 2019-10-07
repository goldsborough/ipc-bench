#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common/common.h"
#include "shm-sync-common.h"

void communicate(struct Sync* sync,
                 struct Arguments* args
                 ) {
    // Buffer into which to read data
    void* buffer = malloc(args->size);

    sync_notify(sync->mutex);

    //printf("4\n");

    for (; args->count > 0; --args->count) {
        sync_wait(sync->mutex);

        //printf("5\n");

        // Read from memory
        memcpy(buffer, sync->shared_memory, args->size);
        // Write back
        memset(sync->shared_memory, '2', args->size);

        //printf("6\n");

        sync_notify(sync->mutex);

        //printf("7\n");
    }

    free(buffer);
}

int main(int argc, char* argv[]) {
    // The synchronization object
    struct Sync sync;

    // Fetch command-line arguments
    struct Arguments args;
    parse_arguments(&args, argc, argv);
    // need some locking between client and server, server must create shm segment and init mutex before client startup!!!
    // any type of IPC - signals for example SIGUSR1
    sleep(1);
    create_sync(&sync, &args);

    communicate(&sync, &args);

    cleanup(&sync);

    return EXIT_SUCCESS;
}
