#include <assert.h>

#include "common/utility.h"
#include "tssx/connection-table.h"
#include "tssx/connection.h"
#include "vector/vector.h"

void table_setup(ConnectionTable* table) {
	if (vector_setup(table, 0, sizeof(Connection)) == VECTOR_ERROR) {
		terminate("Error setting up connection-table\n");
	}
}

void table_destroy(ConnectionTable* table) {
	if (vector_destroy(table) == VECTOR_ERROR) {
		terminate("Error destroying up connection-table\n");
	}
}

void table_assign(ConnectionTable* table,
									size_t index,
									Connection* connection) {
	if (vector_assign(table, index, connection) == VECTOR_ERROR) {
		terminate("Error assigning to connection-table\n");
	}
}

void table_push_back(ConnectionTable* table, Connection* connection) {
	if (vector_push_back(table, connection) == VECTOR_ERROR) {
		terminate("Error pushing back in connection-table\n");
	}
}

void table_safe_remove(ConnectionTable* table, size_t index) {
	Connection empty_connection = CONNECTION_INITIALIZER;
	assert(table != NULL);

	if (vector_assign(table, index, &empty_connection) == VECTOR_ERROR) {
		terminate("Error assigning connection-table entry to safe value");
	}
}

Connection* table_get(ConnectionTable* table, size_t index) {
	void* connection;

	if ((connection = vector_get(table, index)) == NULL) {
		terminate("Error getting connection");
	}

	return (Connection*)connection;
}
