#ifndef BRIDGE_H
#define BRIDGE_H

#include <stdbool.h>

#include "tssx/definitions.h"
#include "tssx/session-table.h"

/******************** DEFINITIONS ********************/

// Included from reverse-map to avoid duplicate definitions
// typedef int key_t;

struct Session;

/******************** STRUCTURES ********************/

// clang-format off
typedef struct Bridge {
  SessionTable session_table;
} Bridge;
// clang-format on

extern Bridge bridge;

/******************** INTERFACE ********************/

void bridge_setup(Bridge* bridge);
void bridge_destroy(Bridge* bridge);

bool bridge_is_initialized(const Bridge* bridge);

void bridge_add_user(Bridge* bridge);

void bridge_insert(Bridge* bridge, int fd, struct Session* session);
void bridge_free(Bridge* bridge, int fd);

struct Session* bridge_lookup(Bridge* bridge, int fd);
bool bridge_has_connection(Bridge* bridge, int fd);

/******************** PRIVATE ********************/

extern signal_handler_t old_sigint_handler;
extern signal_handler_t old_sigterm_handler;
extern signal_handler_t old_sigabrt_handler;

void _setup_exit_handling();
void _setup_signal_handler(int signal_number);

void _bridge_signal_handler();
void _bridge_signal_handler_for(int signal_number,
																signal_handler_t old_handler);
void _bridge_exit_handler();

#endif /* BRIDGE_H */
