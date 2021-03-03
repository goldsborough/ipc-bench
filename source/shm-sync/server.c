#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common/common.h"
#include "shm-sync-common.h"

void communicate(struct Sync* sync,
                 struct Arguments* args
                 ) {
    struct Benchmarks bench;
    int message;
    void* buffer = malloc(args->size);

    // Wait for signal from client
    sync_wait(sync->mutex);
    setup_benchmarks(&bench);

    for (message = 0; message < args->count; ++message) {
        bench.single_start = now();

        //printf("1\n");

        // Write into the memory
        memset(sync->shared_memory, '1', args->size);

        sync_notify(sync->mutex);
        sync_wait(sync->mutex);

        //printf("2\n");

        // Read
        memcpy(buffer, sync->shared_memory, args->size);

        sync_notify(sync->mutex);

        //printf("3\n");

        benchmark(&bench);
    }

    evaluate(&bench, args);
    free(buffer);
}

int main(int argc, char* argv[]) {
    // The synchronization object
    struct Sync sync;

    // Fetch command-line arguments
    struct Arguments args;
    parse_arguments(&args, argc, argv);

    create_sync(&sync, &args);

    communicate(&sync, &args);

    cleanup(&sync);

    return EXIT_SUCCESS;
}
