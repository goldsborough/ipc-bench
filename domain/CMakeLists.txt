add_executable(domain-client client.c ${IPC_BENCH_COMMON_SOURCES})
add_executable(domain-server server.c  ${IPC_BENCH_COMMON_SOURCES})
add_executable(domain domain.c ${IPC_BENCH_COMMON_SOURCES})
