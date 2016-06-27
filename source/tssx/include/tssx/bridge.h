#ifndef BRIDGE_H
#define BRIDGE_H

#include "bitset/bitset.h"
#include "tssx/connection-table.h"
#include "tssx/free-list.h"

// By how much to shift keys negatively
#define KEY_OFFSET -100

#define BRIDGE_INITIALIZER \
	{ CONNECTION_TABLE_INITIALIZER, FREE_LIST_INITIALIZER, BITSET_INITIALIZER }

typedef int key_t;

typedef struct Bridge {
	ConnectionTable table;
	FreeList free_list;
	BitSet occupied;
} Bridge;

void bridge_setup(Bridge* bridge);
void bridge_destroy(Bridge* bridge);

bool bridge_is_initialized(const Bridge* bridge);
bool bridge_is_empty(const Bridge* bridge);

key_t bridge_insert(Bridge* bridge, struct Connection* connection);
void bridge_remove(Bridge* bridge, key_t key);
struct Connection* bridge_lookup(Bridge* bridge, key_t key);

size_t index_for(key_t key);

#endif /* BRIDGE_H */
