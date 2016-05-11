#include <stdio.h>
#include <sys/signal.h>
#include "common/common.h"

void communicate(struct sigaction* signal_action, struct Arguments* args) {
	// Tell the sever we can go
	notify_server();

	for (; args->count > 0; --args->size) {
		wait_for_signal(signal_action);
		notify_server();
	}
}

int main(int argc, char* argv[]) {
	// For command-line arguments
	struct Arguments args;
	struct sigaction signal_action;

	parse_arguments(&args, argc, argv);
	setup_client_signals(&signal_action);

	communicate(&signal_action, &args);
}
