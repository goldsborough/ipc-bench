#ifndef SOCKET_OVERRIDES_H
#define SOCKET_OVERRIDES_H

#include <poll.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>

#include "tssx/definitions.h"

/******************** DEFINITIONS ********************/

typedef int (*real_poll_t)(struct pollfd[], nfds_t, int);

typedef int (*real_select_t)(
		int, fd_set *, fd_set *, fd_set *, struct timeval *);

typedef struct pollfd pollfd;

typedef atomic_uint_fast32_t ready_count_t;

struct Connection;
struct Vector;

typedef struct PollEntry {
	struct Connection *connection;
	pollfd *poll_pointer;
} PollEntry;

/******************** REAL FUNCTIONS ********************/

int real_poll(pollfd fds[], nfds_t nfds, int timeout);

/******************** OVERRIDES ********************/

int poll(pollfd fds[], nfds_t number, int timeout);

/******************** HELPERS ********************/

extern const short operation_map[2];

void _partition(struct Vector *tssx_fds,
								struct Vector *other_fds,
								pollfd fds[],
								nfds_t number);

PollEntry _create_entry(pollfd *poll_pointer);

int _other_poll(struct Vector *other_fds, int timeout);
int _tssx_poll(struct Vector *tssx_fds, int timeout);

bool _check_ready(PollEntry *entry, Operation operation);
bool _waiting_for(PollEntry *entry, Operation operation);
bool _tell_that_ready_for(PollEntry *entry, Operation operation);

bool _timeout_elapsed(size_t start, size_t timeout);

/******************** "POLYMORPHIC" FUNCTIONS ********************/

bool _ready_for(struct Connection *entry, Operation operation);

#endif /* SOCKET_OVERRIDES_H */
