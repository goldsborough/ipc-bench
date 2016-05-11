#ifndef IPC_BENCH_MQ_COMMON_H
#define IPC_BENCH_MQ_COMMON_H

#include <sys/msg.h>

// Note that 2048 seems to the maximum allowed
// size on my system. There is no way to adjust
// this value, unfortunately (can only adjust
// the maximum amount of messages allowed in the
// queue, but not change the size of messages)
#define MESSAGE_TYPE 69
#define MESSAGE_SIZE 2048

struct Message {
	long type;
	char buffer[MESSAGE_SIZE];
};

struct Arguments;

void setup_message(struct Message* message);

#endif /* IPC_BENCH_MQ_COMMON_H */
