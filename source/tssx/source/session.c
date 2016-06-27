#include <assert.h>
#include <stdlib.h>

#include "tssx/connection.h"
#include "tssx/session.h"

bool session_is_valid(Session* session) {
	assert(session != NULL);
	return session->socket != -1;
}

bool session_is_invalid(Session* session) {
	return !session_is_valid(session) && session->connection == NULL;
}

bool session_has_connection(Session* session) {
	assert(session != NULL);
	return session->connection != NULL;
}

void session_invalidate(Session* session) {
	assert(session_is_valid(session));
	session->socket = -1;

	if (session->connection) {
		disconnect(session->connection);
		free(session->connection);
		session->connection = NULL;
	}
}
