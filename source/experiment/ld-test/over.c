#include <sys/socket.h>
#include <stdio.h>

int __real_accept(int, struct sockaddr*, int*);

int __wrap_accept(int a, struct sockaddr* f, int* x) {
	printf("Woo!\n");
}
