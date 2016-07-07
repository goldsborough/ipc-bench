#ifndef SESSION_H
#define SESSION_H

#include <stdbool.h>


/******************** DEFINITIONS ********************/

#define INVALID_SESSION \
	{ -1, NULL }

struct Connection;

/******************** STRUCTURES ********************/

typedef struct Session {
	int socket;
	struct Connection* connection;
} Session;

/******************** INTERFACE ********************/

void session_invalidate(Session* session);
bool session_is_invalid(Session* session);
bool session_is_valid(Session* session);
bool session_has_connection(Session* session);

#endif /* SESSION_H */
