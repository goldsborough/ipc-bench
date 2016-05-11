#include <stdlib.h>

#include <string.h>

#include "common/arguments.h"
#include "mq/mq-common.h"

void setup_message(struct Message* message) {
	// Messages in message-queues are associated with
	// a "type", which is simply an identifier for the message
	// kind. This way, we can put different kinds of messages on
	// the queue, but fetch only the ones we want, by passing
	// the type of the message we want to msgrcv().
	message->type = MESSAGE_TYPE;
}
