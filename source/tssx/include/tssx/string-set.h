#ifndef STRING_SET_H
#define STRING_SET_H

#include <stdbool.h>
#include <stddef.h>

/***** DEFINITIONS *****/

#define SS_MINIMUM_CAPACITY 8
#define SS_LOAD_FACTOR 5
#define SS_MINIMUM_THRESHOLD (SS_MINIUM_CAPACITY) * (SS_LOAD_FACTOR)

#define SS_NOT_INSERTED false
#define SS_INSERTED true

#define SS_NOT_FOUND false
#define SS_FOUND true
#define SS_OK true

#define SS_UNINITIALIZED NULL

#define SS_INITIALIZER {0, 0, 0, SS_UNINITIALIZED};

typedef const char* String;

/***** STRUCTURES *****/

typedef struct Node {
	struct Node* next;

	char key[];// flexible arrray

} Node;

typedef struct StringSet {
	size_t size;
	size_t threshold;
	size_t capacity;

	// The node set
	Node** nodes;

} StringSet;

/***** METHODS *****/

void ss_setup(StringSet* set, size_t capacity);
void ss_destroy(StringSet* set);

bool ss_insert(StringSet* set, String key);
bool ss_remove(StringSet* set, String key);
bool ss_contains(StringSet* set, String key);
void ss_clear(StringSet* set);

bool ss_is_empty(StringSet* set);
bool ss_is_initialized(StringSet* set);

/***** PRIVATE *****/

bool _ss_equals(String first, String second);

void _ss_create_if_necessary(StringSet* set);

void _ss_allocate(StringSet* set, size_t capacity);

Node* _ss_create_node(String key, Node* next);

size_t _ss_hash(StringSet* set, String key);

void _ss_resize(StringSet* set);

void _ss_rehash(StringSet* set, Node** old, size_t old_capacity);

#endif /* HASHTABLE_H */
