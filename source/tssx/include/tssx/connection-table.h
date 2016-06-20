#ifndef CONNECTION_TABLE_H
#define CONNECTION_TABLE_H

struct Vector;
struct Connection;

typedef struct Connection Connection;
typedef struct Vector ConnectionTable;

int table_assign(ConnectionTable* table, int index, Connection* connection);
int table_push_back(ConnectionTable* table, Connection* connection);

Connection* table_lookup(ConnectionTable* table, int index);

#endif /* CONNECTION_TABLE_H */
