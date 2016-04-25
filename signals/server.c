#include <signal.h>
#include "../common.h"

void signal_handler(int signal) {
	if (signal == SIGUSR2) {
		printf("Server received SIGUSR2\n");
	}
}

int main(int argc, const char* argv[]) {
	int signal_number;
	struct sigaction signal_action;
	signal_action.sa_flags = SA_RESTART;
	signal_action.sa_handler = signal_handler;
	sigaddset(&signal_action.sa_mask, SIGUSR1);
	sigprocmask(SIG_BLOCK, &signal_action.sa_mask, NULL);

	if (sigaction(SIGUSR2, &signal_action, NULL)) {
		throw("Error registering signal handler");
	}

	sleep(1);

	for (int i = 0; i < 100; ++i) {
		wait_for_signal(&signal_action);

		printf("Server: %d\n", signal_number);

		// Send SIGUSR1 to all processes in this process'
		// process group (the 0)
		kill(0, SIGUSR2);
	}
}
