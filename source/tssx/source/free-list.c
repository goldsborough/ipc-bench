#include "tssx/free-list.h"
#include "tssx/vector.h"

int free_list_push(FreeList* table, key_t key) {
	return v_push_back(table, V_CAST(&key));
}

key_t free_list_pop(FreeList* table) {
	key_t key = *((key_t*)v_back(table, sizeof(key_t)));
	v_pop_back(table, sizeof(key_t));

	return key;
}
