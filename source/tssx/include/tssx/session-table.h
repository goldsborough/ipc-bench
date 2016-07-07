#ifndef SESSION_TABLE_H
#define SESSION_TABLE_H

#include <stddef.h>

#include "vector/vector.h"

/******************** DEFINITIONS ********************/

#define SESSION_TABLE_INITIALIZER VECTOR_INITIALIZER

// Enable optimization to not handle vector shrinkage
#define VECTOR_NO_SHRINK

struct Session;
typedef struct Vector SessionTable;

/******************** INTERFACE ********************/

void session_table_setup(SessionTable* table);
void session_table_destroy(SessionTable* table);

void session_table_assign(SessionTable* table,
													size_t index,
													struct Session* session);

void session_table_reserve_back(SessionTable* table);
void session_table_push_back(SessionTable* table, struct Session* session);

struct Session* session_table_get(SessionTable* table, size_t index);

#endif /* SESSION_TABLE_H */
