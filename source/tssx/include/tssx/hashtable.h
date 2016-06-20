#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdbool.h>
#include <stddef.h>

#include "connection.h"

/***** DEFINITIONS *****/

#define HT_MINIMUM_CAPACITY 8
#define HT_LOAD_FACTOR 5
#define HT_MINIMUM_THRESHOLD (HT_MINIUM_CAPACITY) * (HT_LOAD_FACTOR)

#define HT_UPDATED false
#define HT_INSERTED true

#define HT_NOT_FOUND false
#define HT_FOUND true
#define HT_OK true

#define HT_UNINITIALIZED NULL

#define HT_INITIALIZER {0, 0, 0, HT_UNINITIALIZED};

/***** STRUCTURES *****/

typedef struct Node {
	int key;
	Connection connection;

	struct Node* next;

} Node;

typedef struct HashTable {
	size_t size;
	size_t threshold;
	size_t capacity;

	// The node table
	Node** nodes;

} HashTable;

/***** METHODS *****/

void ht_setup(HashTable* table, size_t capacity);
void ht_destroy(HashTable* table);

bool ht_insert(HashTable* table, int key, Connection* connection);
bool ht_contains(HashTable* table, int key);
Connection* ht_get(HashTable* table, int key);

bool ht_remove(HashTable* table, int key);
void ht_clear(HashTable* table);

bool ht_is_empty(HashTable* table);

/***** PRIVATE *****/

void _ht_create_if_necessary(HashTable* table);

void _ht_allocate(HashTable* table, size_t capacity);

Node* _ht_create_node(int key, Connection* connection, Node* next);

size_t _ht_hash(HashTable* table, int key);

void _ht_resize(HashTable* table);

void _ht_rehash(HashTable* table, Node** old, size_t old_capacity);

#endif /* HASHTABLE_H */
