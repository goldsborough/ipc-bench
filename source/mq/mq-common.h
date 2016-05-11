#ifndef IPC_BENCH_MQ_COMMON_H
#define IPC_BENCH_MQ_COMMON_H

#include <sys/msg.h>

// Note that 2048 seems to the maximum allowed
// size on my system. There is no way to adjust
// this value, unfortunately (can only adjust
// the maximum amount of messages allowed in the
// queue, but not change the size of messages)
#define MAXIMUM_MESSAGE_SIZE 2048

#define CLIENT_MESSAGE 1
#define SERVER_MESSAGE 2

struct Message {
	// The message type
	long type;

	// Flexible array (must be the last member)
	// https://en.wikipedia.org/wiki/Flexible_array_member
	char buffer[];
};

struct Arguments;

struct Message* create_message(struct Arguments* args);

#endif /* IPC_BENCH_MQ_COMMON_H */
