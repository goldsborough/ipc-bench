#include <assert.h>
#include <signal.h>
#include <stdlib.h>

#include "common/utility.h"
#include "tssx/bridge.h"
#include "tssx/connection.h"
#include "tssx/session.h"

Bridge bridge = BRIDGE_INITIALIZER;

static signal_handler_t old_sigint_handler = NULL;
static signal_handler_t old_sigterm_handler = NULL;

/******************** INTERFACE ********************/

void bridge_setup(Bridge* bridge) {
	assert(bridge != NULL);
	assert(!bridge_is_initialized(bridge));

	session_table_setup(&bridge->session_table);
	free_list_setup(&bridge->free_list);

	_setup_exit_handling();
}

void bridge_destroy(Bridge* bridge) {
	assert(bridge != NULL);

	// Invalidate (disconnect) all sessions
	VECTOR_FOR_EACH(&bridge->session_table, iterator) {
		Session* session = (Session*)iterator_get(&iterator);
		if (session_is_valid(session)) {
			session_invalidate(session);
		}
	}

	session_table_destroy(&bridge->session_table);
	free_list_destroy(&bridge->free_list);
}

bool bridge_is_initialized(const Bridge* bridge) {
	return vector_is_initialized(&bridge->session_table);
}

bool bridge_is_empty(const Bridge* bridge) {
	return vector_is_empty(&bridge->session_table);
}

int bridge_deduce_file_descriptor(Bridge* bridge, int key) {
	if (key < TSSX_KEY_OFFSET) {
		return key;
	} else {
		return bridge_lookup(bridge, key)->socket;
	}
}

void bridge_add_user(Bridge* bridge) {
	VECTOR_FOR_EACH(&bridge->session_table, iterator) {
		Session* session = (Session*)iterator_get(&iterator);
		if (session->connection) {
			connection_add_user(session->connection);
		}
	}
}

key_t bridge_generate_key(Bridge* bridge) {
	key_t key;

	if (!bridge_is_initialized(bridge)) {
		bridge_setup(bridge);
	}

	if (free_list_is_empty(&bridge->free_list)) {
		key = bridge->session_table.size + TSSX_KEY_OFFSET;
		session_table_reserve_back(&bridge->session_table);
	} else {
		key = free_list_pop(&bridge->free_list);
	}

	return key;
}

void bridge_insert(Bridge* bridge, key_t key, Session* session) {
	Session* current_session;
	assert(bridge_is_initialized(bridge));

	// First some sanity checks that this session we're assigning at is not valid
	current_session = session_table_get(&bridge->session_table, index_for(key));
	assert(session_is_invalid(current_session));

	session_table_assign(&bridge->session_table, index_for(key), session);
}

void bridge_free(Bridge* bridge, key_t key) {
	Session* session;

	session = session_table_get(&bridge->session_table, index_for(key));
	session_invalidate(session);
	free_list_push(&bridge->free_list, key);
}

Session* bridge_lookup(Bridge* bridge, key_t key) {
	return session_table_get(&bridge->session_table, index_for(key));
}

size_t index_for(key_t key) {
	assert(key >= TSSX_KEY_OFFSET);
	return key - TSSX_KEY_OFFSET;
}

/******************** PRIVATE ********************/

void _setup_exit_handling() {
	_setup_signal_handler(SIGINT);
	_setup_signal_handler(SIGTERM);
	atexit(_bridge_exit_handler);
}

void _setup_signal_handler(int signal_number) {
	struct sigaction signal_action, old_action;

	assert(signal_number == SIGINT || signal_number == SIGTERM);

	// Set our function as the signal handling function
	signal_action.sa_handler = _bridge_signal_handler;

	// Don't block any other signals during our exception handler
	sigemptyset(&signal_action.sa_mask);

	// Attempt to restart syscalls after our signal handler
	// (useful only for non-terminating signals)
	signal_action.sa_flags = SA_RESTART;

	if (sigaction(signal_number, &signal_action, &old_action) == -1) {
		throw("Error setting signal handler in bridge");
	}

	if (signal_number == SIGINT) {
		old_sigint_handler = old_action.sa_handler;
	} else {
		old_sigterm_handler = old_action.sa_handler;
	}
}

void _bridge_signal_handler(int signal_number) {
	if (signal_number == SIGINT) {
		_bridge_signal_handler_for(SIGINT, old_sigint_handler);
	} else if (signal_number == SIGTERM) {
		_bridge_signal_handler_for(SIGTERM, old_sigterm_handler);
	}
}

void _bridge_signal_handler_for(int signal_number,
																signal_handler_t old_handler) {
	bridge_destroy(&bridge);

	if (old_sigint_handler != NULL) {
		old_sigint_handler(signal_number);
	}

	// It might be that the old signal handler did something that
	// does not terminate the process. Maybe the user decided to
	// just ignore all SIGINT signals. However, if he does, the user
	// program might be able to go on, but we definitely can't because
	// we already destroyed our bridge (all the connection segments).
	// Also, we *have* to destroy the bridge before calling the old handler
	// because if the user does in fact exit (which is most likely), then
	// we want the bridge destroyed first. So we have no choice but to terminate
	// the program irrespective of the user's handler's decision.
	exit(EXIT_FAILURE);
}

void _bridge_exit_handler() {
	bridge_destroy(&bridge);
}
