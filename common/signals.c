#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "common/signals.h"
#include "common/utility.h"

void signal_handler(int _) {
}


void setup_signals(struct sigaction *signal_action, int flags) {
	// Let system calls restart when it
	// was interrupted by a signal
	signal_action->sa_flags = SA_RESTART;

	// Clear all flags
	sigemptyset(&signal_action->sa_mask);

	if (flags & BLOCK_USR1) {
		sigaddset(&signal_action->sa_mask, SIGUSR1);
	}

	if (flags & BLOCK_USR2) {
		sigaddset(&signal_action->sa_mask, SIGUSR2);
	}

	// Change signal mask
	sigprocmask(SIG_BLOCK, &signal_action->sa_mask, NULL);

	// Clear all flags again
	sigemptyset(&signal_action->sa_mask);

	// Now ignore the other signals
	signal_action->sa_handler = SIG_IGN;

	if (!(flags & BLOCK_USR1)) {
		// Set signal handler
		if (sigaction(SIGUSR1, signal_action, NULL)) {
			throw("Error registering SIGUSR1 signal handler for server");
		}
	}

	if (!(flags & BLOCK_USR2)) {
		// Set signal handler
		if (sigaction(SIGUSR2, signal_action, NULL)) {
			throw("Error registering SIGUSR2 signal handler for server");
		}
	}
}

void setup_parent_signals() {
	struct sigaction signal_action;
	setup_signals(&signal_action, IGNORE_USR1 | IGNORE_USR2);
}

void setup_server_signals(struct sigaction *signal_action) {
	setup_signals(signal_action, BLOCK_USR1 | IGNORE_USR2);
	usleep(1000);
}

void setup_client_signals(struct sigaction *signal_action) {
	setup_signals(signal_action, IGNORE_USR1 | BLOCK_USR2);
	usleep(1000);
}

void client_signal() {
	kill(0, SIGUSR1);
}

void server_signal() {
	kill(0, SIGUSR2);
}

void wait_for_signal(struct sigaction *signal_action) {
	int signal_number;
	sigwait(&signal_action->sa_mask, &signal_number);
}
