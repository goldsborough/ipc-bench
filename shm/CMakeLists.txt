add_executable(shm-client client.c ${IPC_BENCH_COMMON_SOURCES})
add_executable(shm-server server.c ${IPC_BENCH_COMMON_SOURCES})
add_executable(shm shm.c ${IPC_BENCH_COMMON_SOURCES})
