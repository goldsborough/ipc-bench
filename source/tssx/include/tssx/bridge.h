#ifndef BRIDGE_H
#define BRIDGE_H

#include "tssx/bitset.h"
#include "tssx/connection-table.h"
#include "tssx/free-list.h"

typedef int key_t;

typedef struct Bridge {
	ConnectionTable table;
	FreeList free_list;
	BitSet occupied;

} Bridge;

void bridge_setup(Bridge* bridge);
void bridge_destroy(Bridge* bridge);

bool bridge_is_initialized(Bridge* bridge);

key_t bridge_insert(Bridge* bridge, Connection* connection);

void bridge_remove(Bridge* bridge, key_t key);

Connection* bridge_lookup(Bridge* bridge, key_t key);

#endif /* BRIDGE_H */
