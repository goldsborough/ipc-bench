#ifndef BRIDGE_H
#define BRIDGE_H

#include "bitset/bitset.h"
#include "tssx/connection-table.h"
#include "tssx/free-list.h"

/******************** DEFINITIONS ********************/

// By how much to shift keys negatively
#define TSSX_KEY_OFFSET (1000000)

#define BRIDGE_INITIALIZER \
	{ CONNECTION_TABLE_INITIALIZER, FREE_LIST_INITIALIZER, BITSET_INITIALIZER }

typedef int key_t;

/******************** STRUCTURES ********************/

typedef struct Bridge {
	ConnectionTable table;
	FreeList free_list;
	BitSet occupied;
} Bridge;

extern Bridge bridge;

/******************** INTERFACE ********************/

void bridge_setup(Bridge* bridge);
void bridge_destroy(Bridge* bridge);

bool bridge_is_initialized(const Bridge* bridge);
bool bridge_is_empty(const Bridge* bridge);

key_t bridge_generate_key(const Bridge* bridge);

key_t bridge_insert(Bridge* bridge, struct Connection* connection);
void bridge_remove(Bridge* bridge, key_t key);
struct Connection* bridge_lookup(Bridge* bridge, key_t key);

size_t index_for(key_t key);

/******************** PRIVATE ********************/

typedef void (*signal_handler_t)(int);

void _setup_exit_handling();
void _setup_signal_handler(int signal_number);

void _bridge_signal_handler();
void _bridge_signal_handler_for(int signal_number,
																signal_handler_t old_handler);
void _bridge_exit_handler();

#endif /* BRIDGE_H */
