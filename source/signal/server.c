#include <stdio.h>
#include <sys/signal.h>
#include "common/common.h"

void communicate(struct sigaction* signal_action, struct Arguments* args) {
	struct Benchmarks bench;
	int message;

	wait_for_signal(signal_action);
	setup_benchmarks(&bench);

	for (message = 0; message < args->count; ++message) {
		bench.single_start = now();

		notify_client();
		wait_for_signal(signal_action);

		benchmark(&bench);
	}

	// "Ignore" the size
	args->size = 1;
	evaluate(&bench, args);
}

int main(int argc, char* argv[]) {
	// For command-line arguments
	struct Arguments args;
	struct sigaction signal_action;

	parse_arguments(&args, argc, argv);
	setup_server_signals(&signal_action);

	communicate(&signal_action, &args);
}
