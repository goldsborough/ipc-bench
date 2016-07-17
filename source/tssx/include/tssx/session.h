#ifndef SESSION_H
#define SESSION_H

#include <stdbool.h>


/******************** DEFINITIONS ********************/

#define INVALID_SESSION \
	{ NULL }

struct Connection;

/******************** STRUCTURES ********************/

// clang-format off
typedef struct Session {
	struct Connection* connection;
} Session;
// clang-format on

/******************** INTERFACE ********************/

void session_setup(Session* session);

bool session_has_connection(const Session* session);
void session_invalidate(Session* session);

#endif /* SESSION_H */
