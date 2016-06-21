#define __STDC_WANT_LIB_EXT1__ 1

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "tssx/vector.h"

int v_setup(Vector* vector, size_t capacity, size_t element_size) {
	assert(vector != NULL);
	assert(!v_is_initialized(vector));

	if (vector == NULL) return V_ERROR;
	if (v_is_initialized(vector)) return V_ERROR;

	vector->size = 0;
	vector->capacity = MAX(V_MINIMUM_CAPACITY, capacity);
	vector->data = malloc(capacity * element_size);

	return vector->data ? V_SUCCESS : V_ERROR;
}

int v_destroy(Vector* vector) {
	assert(vector != NULL);
	assert(v_is_initialized(vector));

	if (vector == NULL) return V_ERROR;
	if (!v_is_initialized(vector)) return V_ERROR;

	free(vector->data);

	return V_SUCCESS;
}

// Insertion
int v_push_back(Vector* vector, void* element, size_t element_size) {
	void* offset;
	int return_code;

	assert(vector != NULL);
	assert(element != NULL);
	assert(element_size > 0);

	if (vector->size == vector->capacity) {
		if (_v_resize(vector, element_size) == V_ERROR) {
			return V_ERROR;
		}
	}

	_v_assign(vector, vector->size, element, element_size);

	++vector->size;

	return V_SUCCESS;
}

int v_push_front(Vector* vector, void* element, size_t element_size) {
	return v_insert(vector, 0, element, element_size);
}

int v_insert(Vector* vector, size_t index, void* element, size_t element_size) {
	void* offset;

	assert(vector != NULL);
	assert(element != NULL);
	assert(element_size > 0);
	assert(index < vector->size);

	if (vector == NULL) return V_ERROR;
	if (element == NULL) return V_ERROR;
	if (element_size == 0) return V_ERROR;
	if (index >= vector->size) return V_ERROR;

	if (vector->size == vector->capacity) {
		if (_v_resize(vector, element_size) == V_ERROR) {
			return V_ERROR;
		}
	}

	// Move other elements to the right
	offset = _v_offset(vector, index, element_size);
	if (_v_move_right(vector, index, offset, element_size) == V_ERROR) {
		return V_ERROR;
	}

	// Insert the element
	memcpy(offset, element, element_size);
	++vector->size;

	return V_SUCCESS;
}

int v_assign(Vector* vector, size_t index, void* element, size_t element_size) {
	void* offset;

	assert(vector != NULL);
	assert(element != NULL);
	assert(element_size > 0);
	assert(index < vector->size);

	if (vector == NULL) return V_ERROR;
	if (element == NULL) return V_ERROR;
	if (element_size == 0) return V_ERROR;
	if (index >= vector->size) return V_ERROR;

	_v_assign(vector, index, element, element_size);

	return V_SUCCESS;
}

// Deletion
int v_pop_back(Vector* vector, size_t element_size) {
	assert(vector != NULL);
	assert(element_size > 0);
	assert(vector->size > 0);

	if (vector == NULL) return V_ERROR;
	if (element_size == 0) return V_ERROR;

#ifndef V_NO_SHRINK
	if (--vector->size == vector->capacity / 4) {
		_v_resize(vector, element_size);
	}
#endif

	return V_SUCCESS;
}

int v_pop_front(Vector* vector, size_t element_size) {
	return v_remove(vector, 0, element_size);
}

int v_remove(Vector* vector, size_t index, size_t element_size) {
	assert(vector != NULL);
	assert(element_size > 0);
	assert(index < vector->size);

	if (vector == NULL) return V_ERROR;
	if (element_size == 0) return V_ERROR;
	if (index >= vector->size) return V_ERROR;

	// Just overwrite
	_v_move_left(vector, index, element_size);

#ifndef V_NO_SHRINK
	if (--vector->size == vector->capacity / 4) {
		_v_resize(vector, element_size);
	}
#endif

	return V_SUCCESS;
}

// Lookup
void* v_get(Vector* vector, size_t index, size_t element_size) {
	assert(vector != NULL);
	assert(element_size > 0);
	assert(index < vector->size);

	if (vector == NULL) return NULL;
	if (element_size == 0) return NULL;
	if (index >= vector->size) return NULL;

	return _v_offset(vector, index, element_size);
}

void* v_front(Vector* vector, size_t element_size) {
	return v_get(vector, 0, element_size);
}

void* v_back(Vector* vector, size_t element_size) {
	return v_get(vector, vector->size - 1, element_size);
}

// Information

bool v_is_initialized(Vector* vector) {
	return vector->data != NULL;
}

size_t v_free_space(Vector* vector) {
	return vector->capacity - vector->size;
}

bool v_is_empty(Vector* vector) {
	return vector->size == 0;
}

/***** PRIVATE *****/

size_t _v_free_bytes(Vector* vector, size_t element_size) {
	return v_free_space(vector) * element_size;
}

void* _v_offset(Vector* vector, size_t index, size_t element_size) {
	return vector->data + (index * element_size);
}

void _v_assign(Vector* vector,
							 size_t index,
							 void* element,
							 size_t element_size) {
	// Insert the element
	void* offset = _v_offset(vector, index, element_size);
	memcpy(offset, element, element_size);
}

int _v_move_right(Vector* vector,
									size_t index,
									void* offset,
									size_t element_size) {
	// How many to move to the right
	size_t elements_in_bytes = (vector->size - index) * element_size;

#ifdef __STDC_LIB_EXT1__
	size_t right_capacity_in_bytes =
			(vector->capacity - (index + 1)) * element_size;
	// clang-format off
	return memmove_s(
		offset + element_size,
		right_capacity_in_bytes,
		offset,
		elements_in_bytes
	);
// clang-format on
#else
	memmove(offset + element_size, offset, elements_in_bytes);
	return V_SUCCESS;
#endif
}

void _v_move_left(Vector* vector, size_t index, size_t element_size) {
	size_t right_elements_in_bytes;
	void* offset;

	// The offset into the memory
	offset = _v_offset(vector, index, element_size);

	// How many to move to the left
	right_elements_in_bytes = (vector->size - index - 1) * element_size;

	memmove(offset, offset + element_size, right_elements_in_bytes);
}

int _v_resize(Vector* vector, size_t element_size) {
	return _v_resize_to_capacity(vector, vector->size * 2 * element_size);
}

int _v_resize_to_capacity(Vector* vector, size_t new_capacity) {
	assert(vector != NULL);

	if (new_capacity < V_MINIMUM_CAPACITY) return V_SUCCESS;

	void* old = vector->data;
	if ((vector->data = malloc(new_capacity)) == NULL) {
		return V_ERROR;
	}

#ifdef __STDC_LIB_EXT1__
	if (memcpy_s(vector->data, new_capacity, old, vector->size) != 0) {
		return V_ERROR;
	}
#else
	memcpy(vector->data, old, vector->size);
#endif

	vector->capacity = new_capacity;

	free(old);

	return V_SUCCESS;
}
