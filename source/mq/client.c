#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/common.h"
#include "mq/mq-common.h"

void communicate(int mq, struct Arguments* args) {
	struct Message* message;

	message = create_message(args);

	for (; args->count > 0; --args->count) {
		// Fetch a message from the queue.
		// Arguments:
		// 1. The message-queue identifier.
		// 2. A pointer to our message, which may be any
		//    data-structure, as long as its first member is of
		//    type long and holds the type of the message. As
		//    such, there exists a template data-structure
		//    struct msbgf { long mtype; char mtext[1]; };
		//    to demonstrate how such a message should look like.
		//    I.e., it has its type member and exactly one more
		//    member, pointing to the data to send.
		// 3. The size of the message, excluding the type member.
		// 4. The message type/kind to fetch. The point is, that
		//    you can put many kinds of messages on the queue, but
		//    in this case only the first one with type = SERVER_MESSAGE
		//    will be retrieved. By passing 0, we could say that we
		//    want *any* kind of message.
		// 5. Flags, which we don't need.
		if (msgrcv(mq, message, args->size, SERVER_MESSAGE, 0) < args->size) {
			throw("Error receiving on client-side");
		}

		message->type = CLIENT_MESSAGE;
		memset(message->buffer, '1', args->size);

		// Same parameters as msgrcv, but no message-type
		// (because it is determined by the message's member)
		// Note that msgsend only returns 0 on success, not the
		// number of bytes, so we don't have to check for < args->size
		if (msgsnd(mq, message, args->size, 0) == -1) {
			throw("Error sending on client-side");
		}
	}

	free(message);
}

int create_mq() {
	int mq;
	key_t key;

	client_once(WAIT);

	// Generate a key for the message-queue
	key = generate_key("mq");

	// msgget is the call to create or retrieve a message queue.
	// It is identified by its unique key, which we generated above
	// and which we pass as first argument. 0666 are the
	// read+write permissions for user, group and world.
	if ((mq = msgget(key, 0666)) == -1) {
		throw("Error retrieving message-queue on client-side");
	}

	return mq;
}

int main(int argc, char* argv[]) {
	// A message-queue is simply identified
	// by a numeric ID
	int mq;

	// For command-line arguments
	struct Arguments args;
	parse_arguments(&args, argc, argv);

	if (args.size > MAXIMUM_MESSAGE_SIZE) {
		args.size = MAXIMUM_MESSAGE_SIZE;
	}

	mq = create_mq();
	communicate(mq, &args);

	return EXIT_SUCCESS;
}
