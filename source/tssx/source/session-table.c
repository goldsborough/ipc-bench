#include <assert.h>

#include "common/utility.h"
#include "tssx/session-table.h"
#include "tssx/session.h"
#include "vector/vector.h"

void session_table_setup(SessionTable* table) {
	assert(table != NULL);
	for (size_t index = 0; index < SESSION_TABLE_SIZE; ++index) {
		Session* session = &(*table)[index];
		session_setup(session);
	}
}

void session_table_destroy(SessionTable* table) {
	assert(table != NULL);
	for (size_t index = 0; index < SESSION_TABLE_SIZE; ++index) {
		Session* session = &(*table)[index];
		assert(!session_has_connection(session));
		session_invalidate(session);
	}
}

void session_table_assign(SessionTable* table, size_t index, Session* session) {
	assert(table != NULL);
	assert(index < SESSION_TABLE_SIZE);
	(*table)[index] = *session;
}

Session* session_table_get(SessionTable* table, size_t index) {
	assert(table != NULL);
	assert(index < SESSION_TABLE_SIZE);
	return &(*table)[index];
}
