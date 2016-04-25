add_executable(mmap-server server.c ${IPC_BENCH_COMMON_SOURCES})
add_executable(mmap-client client.c ${IPC_BENCH_COMMON_SOURCES})
add_executable(mmap mmap.c ${IPC_BENCH_COMMON_SOURCES})
