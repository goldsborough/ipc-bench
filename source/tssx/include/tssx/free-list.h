#ifndef FREE_LIST_H
#define FREE_LIST_H

#include <stdbool.h>
#include <stddef.h>

struct Vector;

typedef struct Vector FreeList;
typedef int key_t;

void free_list_setup(FreeList* list);
void free_list_destroy(FreeList* list);

void free_list_push(FreeList* list, key_t key);
key_t free_list_pop(FreeList* list);

bool free_list_is_empty(FreeList* list);

#endif /* FREE_LIST_H */
