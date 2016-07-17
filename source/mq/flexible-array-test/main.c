#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#include "common/common.h"

struct X {
	long type;
	char buffer[];
};


int main() {
	int fd = ftok(__FILE__, 'x');

	int mq = msgget(key, IPC_CREAT | 0666);

	const int length = 1234;

	struct X* x = malloc(sizeof *x + length * sizeof x->buffer[0]);

	if (mq == -1) throw("Faulty message queue");

	strcpy(x->buffer, "Hello, World!");

	x->type = 69;

	if (msgsnd(mq, x, length, 0) == -1) {
		throw("Error sending");
	}

	if (msgrcv(mq, x, length, 0, 0) < length) {
		throw("Error receiving on server-side");
	}

	printf("%s\n", x->buffer);
}
