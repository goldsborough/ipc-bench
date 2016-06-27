#include "tssx/free-list.h"
#include "common/utility.h"
#include "tssx/bridge.h"
#include "vector/vector.h"

void free_list_setup(FreeList* list) {
	if (vector_setup(list, 0, sizeof(key_t)) == VECTOR_ERROR) {
		terminate("Error setting up free-list\n");
	}
}

void free_list_destroy(FreeList* list) {
	if (vector_destroy(list) == VECTOR_ERROR) {
		terminate("Error destroying up free-list\n");
	}
}

void free_list_push(FreeList* list, key_t key) {
	if (vector_push_back(list, key) == VECTOR_ERROR) {
		terminate("Error pushing into free-list\n");
	}
}

key_t free_list_pop(FreeList* list) {
	key_t* pointer;
	key_t key;

	if ((pointer = (key_t*)vector_back(list)) == NULL) {
		terminate("Error retrieving back of free-list\n");
	}

	if (vector_pop_back(list) == VECTOR_ERROR) {
		terminate("Error popping back of free-list\n");
	}

	return key;
}

bool free_list_is_empty(FreeList* list) {
	return vector_is_empty(list);
}
