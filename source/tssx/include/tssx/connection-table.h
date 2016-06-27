#ifndef CONNECTION_TABLE_H
#define CONNECTION_TABLE_H

#include <stddef.h>

#define CONNECTION_TABLE_INITIALIZER VECTOR_INITIALIZER

struct Vector;
struct Connection;

typedef struct Vector ConnectionTable;

void table_setup(ConnectionTable* table);
void table_destroy(ConnectionTable* table);

void table_assign(ConnectionTable* table,
									size_t index,
									struct Connection* connection);
void table_push_back(ConnectionTable* table, struct Connection* connection);

struct Connection* table_get(ConnectionTable* table, size_t index);

#endif /* CONNECTION_TABLE_H */
