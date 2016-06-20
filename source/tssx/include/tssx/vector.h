#ifndef VECTOR_H
#define VECTOR_H

#include <stdbool.h>
#include <stddef.h>

#include "connection.h"

/***** DEFINITIONS *****/

#define V_MINIMUM_CAPACITY 16

#define V_UNINITIALIZED NULL
#define V_INITIALIZER {0, 0, V_UNINITIALIZED};

#define V_CAST(element) ((void*)(element), sizeof(*(element)))
#define V_TIMES(type, number) (sizeof((type)) * (number))

#define V_ERROR -1
#define V_SUCCESS 0

/***** STRUCTURES *****/

typedef struct Vector {
	size_t size;
	size_t capacity;

	void* data;
} Vector;

/***** METHODS *****/

// Constructor / Destructor
int v_setup(Vector* vector, size_t capacity);
int v_destroy(Vector* vector);

// Insertion
int v_push_back(Vector* vector, void* element, size_t element_size);
int v_push_front(Vector* vector, void* element, size_t element_size);
int v_insert(Vector* vector, size_t index, void* element, size_t element_size);

// Deletion
int v_pop_back(Vector* vector, size_t element_size);
int v_pop_front(Vector* vector, size_t element_size);
int v_remove(Vector* vector, size_t index, size_t element_size);

// Lookup
void* v_get(Vector* vector, size_t index, size_t element_size);
void* v_front(Vector* vector, size_t element_size);
void* v_back(Vector* vector, size_t element_size);

// Information
bool v_is_initialized(Vector* vector);
size_t v_free_space(Vector* vector);
bool v_is_empty(Vector* vector);

/***** PRIVATE *****/

#define MAX(a, b) ((a) > (b) ? (a) : (b))

size_t _v_free_bytes(Vector* vector, size_t element_size);

int _v_move_right(Vector* vector,
									size_t index,
									void* offset,
									size_t element_size);

void _v_move_left(Vector* vector, size_t index, size_t element_size);

int _v_resize(Vector* vector, size_t element_size);
int _v_resize_to_capacity(Vector* vector, size_t new_capacity);

#endif /* VECTOR_H */
