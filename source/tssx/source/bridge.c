#include <assert.h>

#include "tssx/bridge.h"
#include "tssx/connection.h"

void bridge_setup(Bridge* bridge) {
	assert(bridge != NULL);

	table_setup(&bridge->table);
	free_list_setup(&bridge->free_list);
	bitset_setup(&bridge->occupied, 16);
}

void bridge_destroy(Bridge* bridge) {
	assert(bridge != NULL);

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
		key = -bridge->table.size + KEY_OFFSET;
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
	assert(bitset_test(&bridge->occupied, index_for(key)));

	bitset_reset(&bridge->occupied, index_for(key));
	free_list_push(&bridge->free_list, index_for(key));

	if (bridge_is_empty(bridge)) {
		bridge_destroy(bridge);
	}
}

Connection* bridge_lookup(Bridge* bridge, key_t key) {
	// if (!bitset_test(&bridge->occupied, index_for(key))) return NULL;
	assert(bitset_test(&bridge->occupied, index_for(key)));
	return table_get(&bridge->table, index_for(key));
}

size_t index_for(key_t key) {
	return -key + KEY_OFFSET;
}
