#include <assert.h>

#include "tssx/bridge.h"

void bridge_setup(Bridge* bridge) {
	assert(bridge != NULL);

	table_setup(&bridge->table);
	free_list_setup(&bridge->free_list);
	bitset_setup(&bridge->occupied);
}

void bridge_destroy(Bridge* bridge) {
	assert(bridge != NULL);

	table_destroy(&bridge->table);
	free_list_destroy(&bridge->free_list);
	bitset_destroy(&bridge->occupied);
}

bool bridge_is_initialized(Bridge* bridge) {
	return v_is_initialized(&bridge->table);
}

key_t bridge_insert(Bridge* bridge, Connection* connection) {
	key_t key;

	if (!bridge_is_initialized(bridge)) {
		bridge_setup(bridge);
	}

	if (free_list_is_empty(&bridge->free_list)) {
		key = bridge->table.size;
		table_push_back(&bridge->table, connection);
		bit_push_one(&bridge->occupied);
	} else {
		key = free_list_pop(&bridge->free_list);
		table_assign(&bridge->table, key, connection);
		bit_set(&bridge->occupied, key);
	}

	return key;
}

void bridge_remove(Bridge* bridge, key_t key) {
	assert(bit_get(&bridge->occupied, key));

	bit_unset(&bridge->occupied, key);
	free_list_push(&bridge->free_list, key);
}

Connection* bridge_lookup(Bridge* bridge, key_t key) {
	if (!bit_get(&bridge->occupied, key)) return NULL;
	return table_get(&bridge->table, key);
}
