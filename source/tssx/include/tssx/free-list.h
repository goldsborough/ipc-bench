#ifndef FREE_LIST_H
#define FREE_LIST_H

struct Vector;

typedef struct Vector FreeList;
typedef int key_t;

int free_list_push(FreeList* table, key_t key);
key_t free_list_pop(FreeList* table);

#endif /* FREE_LIST_H */
