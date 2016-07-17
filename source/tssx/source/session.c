#include <assert.h>
#include <stdlib.h>

#include "tssx/connection.h"
#include "tssx/session.h"

void session_setup(Session* session) {
	assert(session != NULL);
	session->connection = NULL;
}

bool session_has_connection(const Session* session) {
	assert(session != NULL);
	return session->connection != NULL;
}

void session_invalidate(Session* session) {
	if (!session_has_connection(session)) return;

	disconnect(session->connection);
	free(session->connection);
	session->connection = NULL;
}
