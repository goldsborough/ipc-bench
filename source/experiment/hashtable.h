#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "connection.h"

#define HT_MINIMUM_CAPACITY 8
#define HT_LOAD_FACTOR 5

#define HT_UPDATED 0
#define HT_INSERTED 1

#define HT_NOT_FOUND 0
#define HT_OK 1

typedef struct Node {
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

void ht_setup(HashTable* table, int capacity);
void ht_destroy(HashTable* table);

int ht_insert(HashTable* table, Connection* connection);
Connection* ht_get(HashTable* table, int id);

int ht_remove(HashTable* table, int id);
void ht_clear(HashTable* table);

int ht_is_empty(HashTable* table);

/***** PRIVATE *****/

void ht_allocate(HashTable* table, int capacity);

Node* ht_create_node(Connection* connection, Node* next);

int ht_hash(HashTable* table, int id);

void ht_resize(HashTable* table);

void ht_rehash(HashTable* table, Node** old, int old_capacity);

int ht_id(Node* node);

#endif /* HASHTABLE_H */
