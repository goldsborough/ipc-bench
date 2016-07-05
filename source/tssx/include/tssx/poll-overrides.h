#ifndef SOCKET_OVERRIDES_H
#define SOCKET_OVERRIDES_H

#include <poll.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>

#include "definitions.h"

/******************** DEFINITIONS ********************/

typedef int (*real_poll_t)(struct pollfd[], nfds_t, int);

typedef struct pollfd pollfd;
typedef enum Operation { READ, WRITE } Operation;

typedef atomic_uint_fast32_t ready_count_t;

struct Connection;
struct Vector;

typedef struct PollEntry {
	struct Connection* connection;
	pollfd* poll_pointer;
} PollEntry;

/******************** REAL FUNCTION ********************/

int real_poll(pollfd fds[], nfds_t nfds, int timeout);

/******************** OVERRIDES ********************/

int poll(pollfd fds[], nfds_t number, int timeout);

/******************** HELPERS ********************/

// should _
extern const short operation_map[2];

void partition(struct Vector* tssx_fds,
							 struct Vector* other_fds,
							 pollfd fds[],
							 nfds_t number);

PollEntry create_entry(pollfd* poll_pointer);

int other_poll(struct Vector* other_fds, int timeout);
int tssx_poll(struct Vector* tssx_fds, int timeout);

bool check_ready(PollEntry* entry, Operation operation);
bool waiting_for(PollEntry* entry, Operation operation);
bool ready_for(PollEntry* entry, Operation operation);
bool tell_that_ready_for(PollEntry* entry, Operation operation);

size_t now();
bool timeout_elapsed(size_t start, size_t timeout);

#endif /* SOCKET_OVERRIDES_H */
