#ifndef POLL_OVERRIDES_H
#define POLL_OVERRIDES_H

#include <poll.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

#include "tssx/definitions.h"

/******************** DEFINITIONS ********************/

#define POLL_SIGNAL SIGUSR1
#define BLOCK_FOREVER 0
#define DONT_BLOCK -1

typedef int (*real_poll_t)(struct pollfd[], nfds_t, int);
typedef void *(*thread_function_t)(void *);

typedef struct pollfd pollfd;
typedef atomic_int_fast16_t ready_count_t;

struct Connection;
struct Vector;
struct sigaction;

typedef struct PollEntry {
	struct Connection *connection;
	pollfd *poll_pointer;
} PollEntry;

typedef struct PollTask {
	struct Vector *fds;
	int timeout;
	ready_count_t *ready_count;
} PollTask;

/******************** REAL FUNCTIONS ********************/

int real_poll(pollfd fds[], nfds_t nfds, int timeout);

/******************** OVERRIDES ********************/

int poll(pollfd fds[], nfds_t number, int timeout);

/******************** HELPERS ********************/

extern const short _operation_map[2];

void _partition(struct Vector *tssx_fds,
								struct Vector *other_fds,
								pollfd fds[],
								nfds_t number);

PollEntry _create_entry(pollfd *poll_pointer);

int _start_other_poll_thread(pthread_t *poll_thread, PollTask *task);

int _simple_tssx_poll(struct Vector *tssx_fds, int timeout);

int _concurrent_poll(struct Vector *tssx_fds,
										 struct Vector *other_fds,
										 int timeout);
void _other_poll(PollTask *task);
void _concurrent_tssx_poll(PollTask *task, pthread_t other_thread);

void _setup_tasks(PollTask *other_task,
									PollTask *tssx_task,
									struct Vector *other_fds,
									struct Vector *tssx_fds,
									int timeout,
									ready_count_t *ready_count);

bool _check_ready(PollEntry *entry, Operation operation);
bool _waiting_for(PollEntry *entry, Operation operation);
bool _tell_that_ready_for(PollEntry *entry, Operation operation);

bool _there_was_an_error(ready_count_t *ready_count);
bool _poll_timeout_elapsed(size_t start, int timeout);

int _install_poll_signal_handler(struct sigaction *old_action);
int _restore_old_signal_action(struct sigaction *old_action);
void _poll_signal_handler(int signal_number);

void _cleanup(struct Vector *tssx_fds, struct Vector *other_fds);

/******************** "POLYMORPHIC" FUNCTIONS ********************/

bool _ready_for(struct Connection *entry, Operation operation);

#endif /* POLL_OVERRIDES_H */
