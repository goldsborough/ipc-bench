#ifndef IPC_BENCH_UTILITY_H
#define IPC_BENCH_UTILITY_H

#define true 1
#define false 0

void throw(const char* message);

int generate_key(const char* path);

void nsleep(int nanoseconds);

#endif /* IPC_BENCH_UTILITY_H */
