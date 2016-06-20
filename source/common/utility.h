#ifndef IPC_BENCH_UTILITY_H
#define IPC_BENCH_UTILITY_H

#include <stdbool.h>

void throw(const char* message);

int generate_key(const char* path);

void nsleep(int nanoseconds);

void pin_thread(int where);

#endif /* IPC_BENCH_UTILITY_H */
