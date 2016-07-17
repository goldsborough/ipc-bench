#ifndef SESSION_TABLE_H
#define SESSION_TABLE_H

#include <stdbool.h>
#include <stddef.h>
#include <sys/select.h>

#include "tssx/session.h"

/******************** DEFINITIONS ********************/

#define SESSION_TABLE_SIZE FD_SETSIZE

typedef Session SessionTable[SESSION_TABLE_SIZE];

/******************** INTERFACE ********************/

void session_table_setup(SessionTable* table);
void session_table_destroy(SessionTable* table);

void session_table_assign(SessionTable* table,
													size_t index,
													struct Session* session);
struct Session* session_table_get(SessionTable* table, size_t index);

#endif /* SESSION_TABLE_H */
