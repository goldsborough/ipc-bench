#include "tssx/connection-table.h"
#include "common/utility.h"
#include "tssx/connection.h"
#include "tssx/vector.h"

void table_setup(ConnectionTable* table) {
	if (v_setup(table, 0, sizeof(Connection)) == V_ERROR) {
		terminate("Error setting up connection-table\n");
	}
}

void table_destroy(ConnectionTable* table) {
	if (v_destroy(table) == V_ERROR) {
		terminate("Error destroying up connection-table\n");
	}
}

void table_assign(ConnectionTable* table,
									size_t index,
									Connection* connection) {
	if (v_assign(table, index, V_CAST(connection)) == V_ERROR) {
		terminate("Error assigning to connection-table\n");
	}
}

void table_push_back(ConnectionTable* table, Connection* connection) {
	if (v_push_back(table, V_CAST(connection)) == V_ERROR) {
		terminate("Error pushing back in connection-table\n");
	}
}

Connection* table_lookup(ConnectionTable* table, size_t index) {
	void* connection;

	if ((connection = v_get(table, index, sizeof(Connection))) == NULL) {
		terminate("Error looking up connection");
	}

	return (Connection*)connection;
}
