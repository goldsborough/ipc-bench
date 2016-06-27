#ifndef FREE_LIST_H
#define FREE_LIST_H

#include <stdbool.h>
#include <stddef.h>

#define FREE_LIST_INITIALIZER VECTOR_INITIALIZER

struct Vector;
typedef struct Vector FreeList;

void free_list_setup(FreeList* list);
void free_list_destroy(FreeList* list);

void free_list_push(FreeList* list, int key);
int free_list_pop(FreeList* list);

bool free_list_is_empty(FreeList* list);

#endif /* FREE_LIST_H */
