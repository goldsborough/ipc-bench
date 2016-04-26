#include <stdio.h>
#include <stdlib.h>
#include <zmq.h>

#include "common/common.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <zmq.h>

#include "common/common.h"

void communicate(void* socket, struct Arguments* args) {
	void* buffer = malloc(args->size);

	for (; args->count > 0; --args->count) {
		// Send data to the server (flags = 0)
		if (zmq_send(socket, buffer, args->size, 0) < args->size) {
			throw("Error sending on client-side");
		}

		// Receive data from the server
		if (zmq_recv(socket, buffer, args->size, 0) < args->size) {
			throw("Error receiving on client-side");
		}
	}

	free(buffer);
}

void cleanup(void* context, void* socket) {
	zmq_close(socket);
	zmq_ctx_destroy(context);
}

void* create_socket(void* context) {
	// The socket we will create.
	void* socket;

	// Create a new zmq socket. Note that this is not necessarily
	// a socket in the traditional sense, i.e. that performs
	// network or UNIX-domain I/O. It is just the name ZMQ gives
	// to any of its "connected nodes". The final transmission
	// medium is chosen later, in the call to bind().
	// The second argument to the function specifies the network
	// architecture/pattern of the message queue and this socket's
	// role in that pattern. For example, we will use the simple
	// reply-request model for our server/client pair. In this
	// pattern, there is one (or more) replying node (the server),
	// who thus passes ZMQ_REP, and one (or more) requesting nodes
	// (the client), who passes ZMQ_REQ.
	if ((socket = zmq_socket(context, ZMQ_REQ)) == NULL) {
		throw("Error creating socket");
	}
	// Just like for the call to bind() on the server-side,
	// we now bind (connect) our socket to an address. In
	// doing so, we also tell zmq the transport medium for
	// our connection, in this case a TCP port on localhost
	// with port 6969.
	if (zmq_connect(socket, "tcp://localhost:6969") == -1) {
		throw("Error binding socket to address");
	}

	return socket;
}

void* create_context() {
	void* context;

	// Create a new zmq context, which is the
	// main "control unit" for zmq.
	if ((context = zmq_ctx_new()) == NULL) {
		throw("Error creating ZMQ context");
	}

	return context;
}

int main(int argc, char* argv[]) {
	void* context;
	void* socket;

	// For parsing command-line arguments
	struct Arguments args;
	parse_arguments(&args, argc, argv);

	context = create_context();
	socket = create_socket(context);

	communicate(socket, &args);

	cleanup(context, socket);

	return EXIT_SUCCESS;
}
