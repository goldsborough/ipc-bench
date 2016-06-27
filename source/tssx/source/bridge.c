#include <assert.h>
#include <signal.h>
#include <stdlib.h>

#include "common/utility.h"
#include "tssx/bridge.h"
#include "tssx/connection.h"

Bridge bridge = BRIDGE_INITIALIZER;

static signal_handler_t old_sigint_handler = NULL;
static signal_handler_t old_sigterm_handler = NULL;

/******************** INTERFACE ********************/

void bridge_setup(Bridge* bridge) {
	assert(bridge != NULL);
	assert(!bridge_is_initialized(bridge));

	table_setup(&bridge->table);
	free_list_setup(&bridge->free_list);
	bitset_setup(&bridge->occupied, 16);

	_setup_exit_handling();
}

void bridge_destroy(Bridge* bridge) {
	assert(bridge != NULL);

	for (size_t index = 0; index < bridge->table.size; ++index) {
		if (bitset_test(&bridge->occupied, index)) {
			disconnect(table_get(&bridge->table, index));
		}
	}

	table_destroy(&bridge->table);
	free_list_destroy(&bridge->free_list);
	bitset_destroy(&bridge->occupied);
}

bool bridge_is_initialized(const Bridge* bridge) {
	return vector_is_initialized(&bridge->table);
}

bool bridge_is_empty(const Bridge* bridge) {
	return vector_is_empty(&bridge->table);
}

key_t bridge_insert(Bridge* bridge, Connection* connection) {
	key_t key;

	if (!bridge_is_initialized(bridge)) {
		bridge_setup(bridge);
	}

	if (free_list_is_empty(&bridge->free_list)) {
		key = bridge->table.size + TSSX_KEY_OFFSET;
		table_push_back(&bridge->table, connection);
		bitset_push_one(&bridge->occupied);
	} else {
		assert(!bitset_test(&bridge->occupied, index_for(key)));
		key = free_list_pop(&bridge->free_list);
		table_assign(&bridge->table, index_for(key), connection);
		bitset_set(&bridge->occupied, index_for(key));
	}

	return key;
}

void bridge_remove(Bridge* bridge, key_t key) {
	size_t index;

	assert(bitset_test(&bridge->occupied, index_for(key)));

	index = index_for(key);
	bitset_reset(&bridge->occupied, index);
	free_list_push(&bridge->free_list, index);
	table_safe_remove(&bridge->table, index);
}

Connection* bridge_lookup(Bridge* bridge, key_t key) {
	// if (!bitset_test(&bridge->occupied, index_for(key))) return NULL;
	assert(bitset_test(&bridge->occupied, index_for(key)));
	return table_get(&bridge->table, index_for(key));
}

size_t index_for(key_t key) {
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
	} else {
		exit(EXIT_FAILURE);
	}
}

void _bridge_exit_handler() {
	bridge_destroy(&bridge);
}
