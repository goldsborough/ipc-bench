#include <signal.h>
#include "../common.h"

void signal_handler(int signal) {
	if (signal == SIGUSR1) {
		printf("Client received SIGUSR1\n");
	}
}

int main(int argc, const char* argv[]) {
	int signal_number;
	struct sigaction signal_action;
	signal_action.sa_flags = SA_RESTART;
	signal_action.sa_handler = signal_handler;
	sigaddset(&signal_action.sa_mask, SIGUSR2);
	sigprocmask(SIG_BLOCK, &signal_action.sa_mask, NULL);

	if (sigaction(SIGUSR1, &signal_action, NULL)) {
		throw("Error registering signal handler");
	}
	sleep(1);

	for (int i = 0; i < 100; ++i) {
		// Send SIGUSR1 to all processes in this process'
		// process group
		kill(0, SIGUSR1);

		sigwait(&signal_action.sa_mask, &signal_number);

		printf("Client: %d\n", signal_number);
	}
}
