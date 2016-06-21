#include "tssx/free-list.h"
#include "common/utility.h"
#include "tssx/vector.h"

void free_list_setup(FreeList* list) {
	if (v_setup(list, 0, sizeof(key_t)) == V_ERROR) {
		terminate("Error setting up free-list\n");
	}
}

void free_list_destroy(FreeList* list) {
	if (v_destroy(list) == V_ERROR) {
		terminate("Error destroying up free-list\n");
	}
}

void free_list_push(FreeList* list, key_t key) {
	if (v_push_back(list, V_CAST(&key)) == V_ERROR) {
		terminate("Error pushing into free-list\n");
	}
}

key_t free_list_pop(FreeList* list) {
	key_t* pointer;
	key_t key;

	if ((pointer = (key_t*)v_back(list, sizeof(key_t))) == NULL) {
		terminate("Error retrieving back of free-list\n");
	}

	if (v_pop_back(list, sizeof(key_t)) == V_ERROR) {
		terminate("Error popping back of free-list\n");
	}

	return key;
}

bool free_list_is_empty(FreeList* list) {
	return v_is_empty(list);
}
