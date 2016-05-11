#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "common/signals.h"
#include "common/utility.h"

void signal_handler(int signal_number) {
}


void setup_ignored_signals(struct sigaction *signal_action, int flags) {
	// Now ignore the other signals
	signal_action->sa_handler = signal_handler;

	// Ignore SIGUSR1 ?
	if (!(flags & BLOCK_USR1)) {
		// Set signal handler
		if (sigaction(SIGUSR1, signal_action, NULL)) {
			throw("Error registering SIGUSR1 signal handler for server");
		}
	}

	// Ignore SIGUSR2 ?
	if (!(flags & BLOCK_USR2)) {
		// Set signal handler
		if (sigaction(SIGUSR2, signal_action, NULL)) {
			throw("Error registering SIGUSR2 signal handler for server");
		}
	}
}

void setup_blocked_signals(struct sigaction *signal_action, int flags) {
	signal_action->sa_handler = SIG_DFL;

	// Block SIGUSR1 ?
	if (flags & BLOCK_USR1) {
		sigaddset(&signal_action->sa_mask, SIGUSR1);
	}

	// Block SIGUSR2 ?
	if (flags & BLOCK_USR2) {
		sigaddset(&signal_action->sa_mask, SIGUSR2);
	}

	// Change signal mask
	sigprocmask(SIG_BLOCK, &signal_action->sa_mask, NULL);
}

void setup_signals(struct sigaction *signal_action, int flags) {
	// Let system calls restart when it
	// was interrupted by a signal
	signal_action->sa_flags = SA_RESTART;

	// Clear all flags
	sigemptyset(&signal_action->sa_mask);

	setup_ignored_signals(signal_action, flags);

	// Clear all flags
	sigemptyset(&signal_action->sa_mask);

	setup_blocked_signals(signal_action, flags);
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

void notify_server() {
	kill(0, SIGUSR1);
}

void notify_client() {
	kill(0, SIGUSR2);
}

void wait_for_signal(struct sigaction *signal_action) {
	int signal_number;
	sigwait(&(signal_action->sa_mask), &signal_number);
}

void client_once(int operation) {
	struct sigaction signal_action;
	setup_client_signals(&signal_action);
	if (operation == WAIT) {
		wait_for_signal(&signal_action);
	} else {
		notify_server();
	}
}

void server_once(int operation) {
	struct sigaction signal_action;
	setup_server_signals(&signal_action);
	if (operation == WAIT) {
		wait_for_signal(&signal_action);
	} else {
		notify_client();
	}
}
