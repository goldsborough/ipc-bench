#ifndef IPC_BENCH_UTILITY_H
#define IPC_BENCH_UTILITY_H

#ifdef DEBUG
#include <stdio.h>
#endif /* DEBUG */

#include <stdbool.h>

void throw(const char* message);
void terminate(const char* message);
void print_error(const char* message);

int generate_key(const char* path);

void nsleep(int nanoseconds);

void pin_thread(int where);

#endif /* IPC_BENCH_UTILITY_H */
