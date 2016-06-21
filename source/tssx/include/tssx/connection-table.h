#ifndef CONNECTION_TABLE_H
#define CONNECTION_TABLE_H

#include <stddef.h>

struct Vector;
struct Connection;

typedef struct Connection Connection;
typedef struct Vector ConnectionTable;

void table_setup(ConnectionTable* table);
void table_destroy(ConnectionTable* table);

void table_assign(ConnectionTable* table, size_t index, Connection* connection);
void table_push_back(ConnectionTable* table, Connection* connection);

Connection* table_get(ConnectionTable* table, size_t index);

#endif /* CONNECTION_TABLE_H */
