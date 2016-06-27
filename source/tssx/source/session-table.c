#include <assert.h>

#include "common/utility.h"
#include "tssx/session-table.h"
#include "tssx/session.h"
#include "vector/vector.h"

void session_table_setup(SessionTable* table) {
	assert(table != NULL);
	if (vector_setup(table, 0, sizeof(Session)) == VECTOR_ERROR) {
		terminate("Error setting up session table\n");
	}
}

void session_table_destroy(SessionTable* table) {
	assert(table != NULL);
	if (vector_destroy(table) == VECTOR_ERROR) {
		terminate("Error destroying up session table\n");
	}
}

void session_table_assign(SessionTable* table, size_t index, Session* session) {
	assert(table != NULL);
	assert(session_is_valid(session));
	if (vector_assign(table, index, session) == VECTOR_ERROR) {
		terminate("Error assigning to session table\n");
	}
}

void session_table_reserve_back(SessionTable* table) {
	Session session = INVALID_SESSION;
	if (vector_push_back(table, &session) == VECTOR_ERROR) {
		terminate("Error reserving back in session table\n");
	}
}

void session_table_push_back(SessionTable* table, Session* session) {
	assert(table != NULL);
	assert(session_is_valid(session));
	if (vector_push_back(table, session) == VECTOR_ERROR) {
		terminate("Error pushing back in session table\n");
	}
}

Session* session_table_get(SessionTable* table, size_t index) {
	void* session;

	if ((session = vector_get(table, index)) == NULL) {
		terminate("Error getting session");
	}

	return session;
}
