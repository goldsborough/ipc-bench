#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/types.h>

#include "common/common.h"
#include "mq/mq-common.h"

void cleanup(int mq, struct Message* message) {
	// Destroy the message queue.
	// Takes the message-queue ID and an operation,
	// in this case IPC_RMID. The last parameter, for
	// options and other information, can be set to
	// NULL for the purpose of removing the queue.
	if (msgctl(mq, IPC_RMID, NULL) == -1) {
		throw("Error removing message queue");
	}

	free(message);
}


void communicate(int mq, struct Arguments* args) {
	struct Benchmarks bench;
	struct Message* message;
	int index;

	message = create_message(args);
	setup_benchmarks(&bench);

	for (index = 0; index < args->count; ++index) {
		bench.single_start = now();

		// Messages in message-queues are associated with
		// a "type", which is simply an identifier for the message
		// kind. This way, we can put different kinds of messages on
		// the queue, but fetch only the ones we want, by passing
		// the type of the message we want to msgrcv().
		message->type = SERVER_MESSAGE;
		memset(message->buffer, '2', args->size);

		// Same parameters as msgrcv, but no message-type
		// (because it is determined by the message's member)
		if (msgsnd(mq, message, args->size, IPC_NOWAIT) == -1) {
			throw("Error sending on server-side");
		}

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
		//    in this case only the first one with type = CLIENT_MESSAGE
		//    will be retrieved. By passing 0, we could say that we
		//    want *any* kind of message. This call will block until
		//    such a message is available in the queue.
		// 5. Flags, which we don't need.
		if (msgrcv(mq, message, args->size, CLIENT_MESSAGE, 0) < args->size) {
			throw("Error receiving on server-side");
		}

		benchmark(&bench);
	}

	// Since the buffer size must be fixed
	// args->size = MESSAGE_SIZE;
	evaluate(&bench, args);

	cleanup(mq, message);
}

int create_mq() {
	int mq;

	// Generate a key for the message-queue
	key_t key = generate_key("mq");

	// msgget is the call to create or retrieve a message queue.
	// It is identified by its unique key, which we generated above
	// and which we pass as first argument. The second argument is
	// the set of flags for the message queue. By passing IPC_CREAT,
	// the message-queue will be created if it does not yet exist.
	// 0666 are the read+write permissions for user,
	// group and world.
	if ((mq = msgget(key, IPC_CREAT | 0666)) == -1) {
		throw("Error creating message-queue on server-side");
	}

	// Tell the client it can now get the queue
	server_once(NOTIFY);

	return mq;
}

void limit_message_size(struct Arguments* args) {
	if (args->size > MAXIMUM_MESSAGE_SIZE) {
		args->size = MAXIMUM_MESSAGE_SIZE;
		// clang-format off
		fprintf(
			stderr,
			"Reduced the message size to %d bytes!\n",
			MAXIMUM_MESSAGE_SIZE
		);
		// clang-format on
	}
}

int main(int argc, char* argv[]) {
	// A message-queue is simply identified
	// by a numeric ID
	int mq;

	// For command-line arguments
	struct Arguments args;
	parse_arguments(&args, argc, argv);

	limit_message_size(&args);

	mq = create_mq();
	communicate(mq, &args);

	return EXIT_SUCCESS;
}
