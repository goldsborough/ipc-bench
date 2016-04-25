add_executable(tcp-client client.c tcp-common.c ${IPC_BENCH_COMMON_SOURCES})
add_executable(tcp-server server.c tcp-common.c ${IPC_BENCH_COMMON_SOURCES})
add_executable(tcp tcp.c tcp-common.c ${IPC_BENCH_COMMON_SOURCES})
