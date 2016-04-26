add_executable(tcp-client client.c ${IPC_BENCH_COMMON_SOURCES})
add_executable(tcp-server server.c ${IPC_BENCH_COMMON_SOURCES})
add_executable(tcp tcp.c ${IPC_BENCH_COMMON_SOURCES})
