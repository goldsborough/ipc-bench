#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "connection.h"

/***** DEFINITIONS *****/

#define HT_MINIMUM_CAPACITY 8
#define HT_LOAD_FACTOR 5
#define HT_MINIMUM_THRESHOLD (HT_MINIUM_CAPACITY) * (HT_LOAD_FACTOR)

#define HT_UPDATED 0
#define HT_INSERTED 1

#define HT_NOT_FOUND 0
#define HT_OK 1

#define HT_UNINITIALIZED NULL

#define HT_INITIALIZER {0, 0, 0, HT_UNINITIALIZED};

/***** STRUCTURES *****/

typedef struct Node {
	int key;
	Connection connection;

	struct Node* next;

} Node;

typedef struct HashTable {
	int size;
	int threshold;
	int capacity;

	// The node table
	Node** nodes;

} HashTable;

/***** METHODS *****/

void ht_setup(HashTable* table, int capacity);
void ht_destroy(HashTable* table);

int ht_insert(HashTable* table, int key, Connection* connection);
Connection* ht_get(HashTable* table, int key);

int ht_remove(HashTable* table, int key);
void ht_clear(HashTable* table);

int ht_is_empty(HashTable* table);

/***** PRIVATE *****/

void _ht_create_if_necessary(HashTable* table);

void _ht_allocate(HashTable* table, int capacity);

Node* _ht_create_node(int key, Connection* connection, Node* next);

int _ht_hash(HashTable* table, int key);

void _ht_resize(HashTable* table);

void _ht_rehash(HashTable* table, Node** old, int old_capacity);

#endif /* HASHTABLE_H */
