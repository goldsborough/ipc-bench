#include <stdio.h>
#include <sys/ipc.h>

int __real_ftok(const char*, int);

int __wrap_ftok(const char* string, int id) {
	printf("Woo!\n");
	return __real_ftok(string, id);
}

int main(int argc, char* argv[]) {
	int result = ftok(__FILE__, 69);
	printf("%d\n", result);
}
