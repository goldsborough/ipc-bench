#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <time.h>
#define __USE_GNU
#include <pthread.h>
#include <sched.h>

#include "common/utility.h"

void throw(const char* message) {
	perror(message);
	exit(EXIT_FAILURE);
}

int generate_key(const char* path) {
	// Generate a random key from the given file path
	// (inode etc.) plus the arbitrary character
	return ftok(path, 'X');
}

void nsleep(int nanoseconds) {
	struct timespec remaining;
	struct timespec time = {0, nanoseconds};
	nanosleep(&time, &remaining);
}

void pin_thread(int where) {
	// int j;
	// cpu_set_t cpuset;
	// pthread_t thread;
	// thread = pthread_self();
	// CPU_ZERO(&cpuset);
	// CPU_SET(where, &cpuset);
	// pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
	// int s = pthread_getaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
	// if (s != 0) {
	// 	fprintf(stderr, "error: pthread_getaffinity_np");
	// 	exit(-1);
	// }
}
