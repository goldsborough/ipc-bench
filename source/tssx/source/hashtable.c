#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "tssx/hashtable.h"

void ht_setup(HashTable* table, int capacity) {
	assert(table != NULL);

	if (capacity < HT_MINIMUM_CAPACITY) {
		capacity = HT_MINIMUM_CAPACITY;
	}

	_ht_allocate(table, capacity);
	table->size = 0;
}

void ht_destroy(HashTable* table) {
	Node* node;
	Node* next;

	assert(table != NULL);

	for (int i = 0; i < table->capacity; ++i) {
		node = table->nodes[i];
		while (node) {
			next = node->next;
			free(node);
			node = next;
		}
	}

	free(table->nodes);
}

int ht_insert(HashTable* table, int key, Connection* connection) {
	Node* node;
	int index;

	_ht_create_if_necessary(table);

	if (table->size == table->threshold) {
		_ht_resize(table);
	}

	index = _ht_hash(table, key);
	node = table->nodes[index];

	for (; node; node = node->next) {
		if (node->key == key) {
			node->connection = *connection;
			return HT_UPDATED;
		}
	}

	node = _ht_create_node(key, connection, table->nodes[index]);
	table->nodes[index] = node;

	++table->size;

	return HT_INSERTED;
}

Connection* ht_get(HashTable* table, int key) {
	Node* node;
	int index;

	_ht_create_if_necessary(table);

	index = _ht_hash(table, key);
	node = table->nodes[index];

	for (; node; node = node->next) {
		if (node->key == key) {
			return &node->connection;
		}
	}

	return NULL;
}

int ht_remove(HashTable* table, int key) {
	Node* node;
	Node* previous;
	int index;

	_ht_create_if_necessary(table);


	index = _ht_hash(table, key);
	node = table->nodes[index];

	for (previous = NULL; node; previous = node, node = node->next) {
		if (node->key == key) {
			if (previous) {
				previous->next = node->next;
			} else {
				table->nodes[index] = node->next;
			}

			free(node);

			if (--table->size == table->threshold / 4) {
				_ht_resize(table);
			}

			return HT_OK;
		}
	}

	return HT_NOT_FOUND;
}

void ht_clear(HashTable* table) {
	ht_destroy(table);
	_ht_allocate(table, HT_MINIMUM_CAPACITY);
	table->size = 0;
}

int ht_is_empty(HashTable* table) {
	return table->size == 0;
}

/***** PRIVATE *****/

void _ht_create_if_necessary(HashTable* table) {
	assert(table != NULL);
	if (table->nodes == HT_UNINITIALIZED) {
		ht_setup(table, 0);
	}
}

void _ht_allocate(HashTable* table, int capacity) {
	int bytes;

	bytes = sizeof(Node*) * capacity;
	table->nodes = (Node**)malloc(bytes);
	memset(table->nodes, 0, bytes);

	table->capacity = capacity;
	table->threshold = capacity * HT_LOAD_FACTOR;
}

Node* _ht_create_node(int key, Connection* connection, Node* next) {
	Node* node = (Node*)malloc(sizeof(Node));

	node->key = key;
	node->connection = *connection;
	node->next = next;

	return node;
}

int _ht_hash(HashTable* table, int key) {
	const int a = 69;
	const int b = 99;
	const int p = 123;

	return (((a * key) + b) % p) % table->capacity;
}

void _ht_resize(HashTable* table) {
	int old_capacity = table->capacity;
	int new_capacity = table->size * 2;

	if (new_capacity < HT_MINIMUM_CAPACITY) return;

	Node** old = table->nodes;

	_ht_allocate(table, new_capacity);
	_ht_rehash(table, old, old_capacity);

	free(old);
}

void _ht_rehash(HashTable* table, Node** old, int old_capacity) {
	Node* node;
	Node* next;
	int new_index;
	int i;

	for (i = 0; i < old_capacity; ++i) {
		for (node = old[i]; node;) {
			next = node->next;

			new_index = _ht_hash(table, node->key);
			node->next = table->nodes[new_index];
			table->nodes[new_index] = node;

			node = next;
		}
	}
}
