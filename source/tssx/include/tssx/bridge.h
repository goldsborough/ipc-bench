#ifndef BRIDGE_H
#define BRIDGE_H

#include "tssx/free-list.h"
#include "tssx/reverse-map.h"
#include "tssx/session-table.h"

/******************** DEFINITIONS ********************/

#define TSSX_KEY_OFFSET 1000000

#define BRIDGE_INITIALIZER \
	{ SESSION_TABLE_INITIALIZER, FREE_LIST_INITIALIZER }


// Included from reverse-map to avoid duplicate definitions
// typedef int key_t;

struct Session;

/******************** STRUCTURES ********************/

typedef struct Bridge {
	SessionTable session_table;
	FreeList free_list;
	ReverseMap reverse;
} Bridge;

extern Bridge bridge;

/******************** INTERFACE ********************/

void bridge_setup(Bridge* bridge);
void bridge_destroy(Bridge* bridge);

bool bridge_is_initialized(const Bridge* bridge);
bool bridge_is_empty(const Bridge* bridge);

key_t bridge_generate_key(Bridge* bridge);
int bridge_deduce_file_descriptor(Bridge* bridge, int key);

void bridge_add_user(Bridge* bridge);

void bridge_insert(Bridge* bridge, key_t key, struct Session* session);
void bridge_free(Bridge* bridge, key_t key);

struct Session* bridge_lookup(Bridge* bridge, key_t key);
key_t bridge_reverse_lookup(Bridge* bridge, int socket_fd);

size_t index_for(key_t key);

/******************** PRIVATE ********************/

typedef void (*signal_handler_t)(int);

void _setup_exit_handling();
void _setup_signal_handler(int signal_number);

void _bridge_signal_handler();
void _bridge_signal_handler_for(int signal_number,
																signal_handler_t old_handler);
void _bridge_exit_handler();

#endif /* BRIDGE_H */
