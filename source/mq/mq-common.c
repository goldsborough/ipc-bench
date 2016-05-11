#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/arguments.h"
#include "mq/mq-common.h"

struct Message* create_message(struct Arguments* args) {
	struct Message* message;

	// Allocate the message and the flexible array member
	message = malloc(sizeof(*message) + args->size * sizeof(message->buffer[0]));

	return message;
}
