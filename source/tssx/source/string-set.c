#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "tssx/string-set.h"

void ss_setup(StringSet* set, size_t capacity) {
	assert(set != NULL);

	if (capacity < SS_MINIMUM_CAPACITY) {
		capacity = SS_MINIMUM_CAPACITY;
	}

	_ss_allocate(set, capacity);
	set->size = 0;
}

void ss_destroy(StringSet* set) {
	Node* node;
	Node* next;

	assert(set != NULL);

	for (size_t i = 0; i < set->capacity; ++i) {
		node = set->nodes[i];
		while (node) {
			next = node->next;
			free(node);
			node = next;
		}
	}

	free(set->nodes);
}

bool ss_insert(StringSet* set, String key) {
	Node* node;
	size_t index;

	_ss_create_if_necessary(set);

	if (set->size == set->threshold) {
		_ss_resize(set);
	}

	index = _ss_hash(set, key);
	node = set->nodes[index];

	for (; node; node = node->next) {
		if (_ss_equals(node->key, key)) {
			return SS_NOT_INSERTED;
		}
	}

	node = _ss_create_node(key, set->nodes[index]);
	set->nodes[index] = node;

	++set->size;

	return SS_INSERTED;
}

bool ss_contains(StringSet* set, String key) {
	printf("checking for: '%s'\n", key);

	Node* node;
	size_t index;

	if (set->nodes != SS_UNINITIALIZED) {
		index = _ss_hash(set, key);
		for (node = set->nodes[index]; node; node = node->next) {
			if (_ss_equals(node->key, key)) return SS_FOUND;
		}
	}

	return SS_NOT_FOUND;
}

bool ss_remove(StringSet* set, String key) {
	Node* node;
	Node* previous;
	size_t index;

	if (set->nodes == SS_UNINITIALIZED) {
		return SS_NOT_FOUND;
	}

	index = _ss_hash(set, key);
	node = set->nodes[index];

	for (previous = NULL; node; previous = node, node = node->next) {
		if (node->key == key) {
			if (previous) {
				previous->next = node->next;
			} else {
				set->nodes[index] = node->next;
			}

			free(node);

			if (--set->size == set->threshold / 4) {
				_ss_resize(set);
			}

			return SS_OK;
		}
	}

	return SS_NOT_FOUND;
}

void ss_clear(StringSet* set) {
	if (set->nodes == SS_UNINITIALIZED) return;
	ss_destroy(set);
	_ss_allocate(set, SS_MINIMUM_CAPACITY);
	set->size = 0;
}

bool ss_is_empty(StringSet* set) {
	return set->size == 0;
}

bool ss_is_initialized(StringSet* set) {
	return set->nodes != SS_UNINITIALIZED;
}

/***** PRIVATE *****/

bool _ss_equals(String first, String second) {
	return strcmp(first, second) == 0;
}

void _ss_create_if_necessary(StringSet* set) {
	assert(set != NULL);
	if (set->nodes == SS_UNINITIALIZED) {
		ss_setup(set, 0);
	}
}

void _ss_allocate(StringSet* set, size_t capacity) {
	size_t bytes;
	bytes = sizeof(Node*) * capacity;
	set->nodes = (Node**)malloc(bytes);
	memset(set->nodes, 0, bytes);

	set->capacity = capacity;
	set->threshold = capacity * SS_LOAD_FACTOR;
}

Node* _ss_create_node(String key, Node* next) {
	// Allocate the size of the node up to the flexible array member
	// and then also the node itself. This is cool, because only need
	// to delete the node and not the key separately (if we used a char*
	// instead of a flexible array). Also, cache lookup should be improved.
	Node* node = (Node*)malloc(sizeof(Node) + strlen(key));

	strcpy(node->key, key);
	node->next = next;

	return node;
}

size_t _ss_hash(StringSet* set, String key) {
	// djb2 string hashing algorithm
	// sstp://www.cse.yorku.ca/~oz/hash.ssml
	size_t hash;

	for (hash = 5381; *key != '\0'; ++key) {
		// (hash << 5) + hash = hash * 33
		hash = ((hash << 5) + hash) ^ *key;
	}

	return hash % set->capacity;
}

void _ss_resize(StringSet* set) {
	size_t old_capacity = set->capacity;
	size_t new_capacity = set->size * 2;

	if (new_capacity < SS_MINIMUM_CAPACITY) return;

	Node** old = set->nodes;

	_ss_allocate(set, new_capacity);
	_ss_rehash(set, old, old_capacity);

	free(old);
}

void _ss_rehash(StringSet* set, Node** old, size_t old_capacity) {
	Node* node;
	Node* next;
	size_t new_index;
	size_t i;

	for (i = 0; i < old_capacity; ++i) {
		for (node = old[i]; node;) {
			next = node->next;

			new_index = _ss_hash(set, node->key);
			node->next = set->nodes[new_index];
			set->nodes[new_index] = node;

			node = next;
		}
	}
}
