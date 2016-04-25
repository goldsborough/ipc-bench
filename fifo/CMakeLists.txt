add_executable(fifo-client client.c ${IPC_BENCH_COMMON_SOURCES})
add_executable(fifo-server server.c ${IPC_BENCH_COMMON_SOURCES})
add_executable(fifo fifo.c ${IPC_BENCH_COMMON_SOURCES})
