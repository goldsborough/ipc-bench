#include "tssx/connection-table.h"
#include "tssx/connection.h"
#include "tssx/vector.h"

int table_assign(ConnectionTable* table, int index, Connection* connection) {
	return v_assign(table, index, V_CAST(connection));
}

int table_push_back(ConnectionTable* table, Connection* connection) {
	return v_push_back(table, V_CAST(connection));
}

Connection* table_lookup(ConnectionTable* table, int index) {
	return (Connection*)v_get(table, index, sizeof(Connection));
}
