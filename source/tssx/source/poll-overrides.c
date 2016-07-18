#define _GNU_SOURCE

#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>

#include "common/common.h"
#include "tssx/bridge.h"
#include "tssx/poll-overrides.h"
#include "tssx/session.h"
#include "vector/vector.h"

/******************** REAL FUNCTIONS ********************/

int real_poll(pollfd fds[], nfds_t nfds, int timeout) {
	return ((real_poll_t)dlsym(RTLD_NEXT, "poll"))(fds, nfds, timeout);
}

/******************** OVERRIDES ********************/

int poll(pollfd fds[], nfds_t nfds, int timeout) {
	Vector tssx_fds, other_fds;
	int ready_count;

	if (nfds == 0) return 0;

	_partition(&tssx_fds, &other_fds, fds, nfds);

	if (tssx_fds.size == 0) {
		// We are only dealing with normal (non-tssx) fds
		ready_count = real_poll(fds, nfds, timeout);
	} else if (other_fds.size == 0) {
		// We are only dealing with tssx connections
		ready_count = _simple_tssx_poll(&tssx_fds, timeout);
	} else {
		ready_count = _concurrent_poll(&tssx_fds, &other_fds, timeout);
	}

	_cleanup(&tssx_fds, &other_fds);

	return ready_count;
}

/******************** HELPERS ********************/

const short _operation_map[2] = {POLLIN, POLLOUT};

void _partition(Vector* tssx_fds,
								Vector* other_fds,
								pollfd fds[],
								nfds_t number) {
	// Minimum capacity of 32 each
	vector_setup(tssx_fds, 32, sizeof(PollEntry));
	vector_setup(other_fds, 32, sizeof(pollfd));

	for (nfds_t index = 0; index < number; ++index) {
		Session* session = bridge_lookup(&bridge, fds[index].fd);
		if (session_has_connection(session)) {
			PollEntry entry;
			entry.connection = session->connection;
			entry.poll_pointer = &fds[index];
			vector_push_back(tssx_fds, &entry);
		} else {
			vector_push_back(other_fds, &fds[index]);
		}
	}
}

int _concurrent_poll(Vector* tssx_fds, Vector* other_fds, int timeout) {
	PollTask other_task;
	PollTask tssx_task;
	ready_count_t ready_count;
	struct sigaction old_action;
	pthread_t other_thread;

	atomic_init(&ready_count, 0);
	_setup_tasks(&other_task,
							 &tssx_task,
							 other_fds,
							 tssx_fds,
							 timeout,
							 &ready_count);

	if ((_install_poll_signal_handler(&old_action)) == ERROR) {
		return ERROR;
	}

	if (_start_other_poll_thread(&other_thread, &other_task) == ERROR) {
		return ERROR;
	}

	// Note: Will run in this thread, but deals with concurrent polling
	_concurrent_tssx_poll(&tssx_task, other_thread);

	// Theoretically not necessary because we synchronize either through
	// the timeout, or via a change on the ready count (quasi condition variable)
	// Although, note that POSIX requires a join to reclaim resources,
	// unless we detach the thread with pthread_detach to make it a daemon
	pthread_join(other_thread, NULL);
	_restore_old_signal_action(&old_action);

	// Three cases for the ready count
	// An error occurred in either polling, then it is -1.
	// The timeout expired, then it is 0.
	// Either poll found a change, then it is positive.

	return atomic_load(&ready_count);
}

int _start_other_poll_thread(pthread_t* poll_thread, PollTask* task) {
	thread_function_t function = (thread_function_t)_other_poll;

	if (pthread_create(poll_thread, NULL, function, task) != SUCCESS) {
		print_error("Error creating polling thread");
		return ERROR;
	}

	return SUCCESS;
}

void _setup_tasks(PollTask* other_task,
									PollTask* tssx_task,
									Vector* other_fds,
									Vector* tssx_fds,
									int timeout,
									ready_count_t* ready_count) {
	// Setup the tasks
	other_task->fds = other_fds;
	other_task->timeout = timeout;
	other_task->ready_count = ready_count;

	tssx_task->fds = other_fds;
	tssx_task->timeout = timeout;
	tssx_task->ready_count = ready_count;
}

void _other_poll(PollTask* task) {
	int local_ready_count;
	pollfd* raw = task->fds->data;
	size_t size = task->fds->size;

	local_ready_count = real_poll(raw, size, task->timeout);

	// Don't touch anything else if there was an error in the main thread
	if (_there_was_an_error(task->ready_count)) return;

	// Check if there was an error in real_poll, but not EINTR, which
	// would be the signal received when one or more TSSX fd was ready
	if (local_ready_count == ERROR) {
		if (errno != EINTR) {
			atomic_store(task->ready_count, ERROR);
		}
		return;
	}

	// Either there was a timeout (+= 0), or some FDs are ready
	atomic_fetch_add(task->ready_count, local_ready_count);

	pthread_exit(EXIT_SUCCESS);
}

int _simple_tssx_poll(Vector* tssx_fds, int timeout) {
	size_t start = current_milliseconds();
	int ready_count = 0;

	while (!_timeout_elapsed(start, timeout)) {
		// Do a full loop over all FDs
		VECTOR_FOR_EACH(tssx_fds, iterator) {
			PollEntry* entry = (PollEntry*)iterator_get(&iterator);
			if (_check_ready(entry, READ) || _check_ready(entry, WRITE)) {
				++ready_count;
			}
		}

		if (ready_count > 0) break;
	}

	return ready_count;
}

void _concurrent_tssx_poll(PollTask* task, pthread_t other_thread) {
	size_t start = current_milliseconds();
	size_t local_ready_count = 0;

	while (!_timeout_elapsed(start, task->timeout)) {
		// Do a full loop over all FDs
		VECTOR_FOR_EACH(task->fds, iterator) {
			PollEntry* entry = (PollEntry*)iterator_get(&iterator);
			if (_check_ready(entry, READ) || _check_ready(entry, WRITE)) {
				++local_ready_count;
			}
		}

		size_t global_ready_count = atomic_load(task->ready_count);

		// Don't touch if there was an error in the other thread
		if (global_ready_count == ERROR) return;
		if (global_ready_count > 0) break;
		if (local_ready_count > 0) {
			// Send our POLL_SIGNAL to the thread doing real_poll()
			// This will terminate that thread, avoiding weird edge cases
			// where the other thread would block indefinitely if it detects
			// no changes (e.g. on a single fd), even if there were many events
			// on the TSSX buffer in the main thread. Note: signals are a actually a
			// process-wide concept. But because we installed a signal handler, what
			// we can do is have the signal handler be invoked in the *other_thread*
			// argument. If the disposition of the signal were a default (i.e. if we
			// had installed no signal handler) one, such as TERMINATE for SIGQUIT,
			// then that signal would be delivered to all threads, because all threads
			// run in the same process
			pthread_kill(other_thread, POLL_SIGNAL);
			break;
		}
	}

	// Add whatever we have (zero if the timeout elapsed)
	atomic_fetch_add(task->ready_count, local_ready_count);
}

bool _check_ready(PollEntry* entry, Operation operation) {
	if (_waiting_for(entry, operation)) {
		// This here is the polymorphic call (see client/server-overrides)
		if (_ready_for(entry->connection, operation)) {
			_tell_that_ready_for(entry, operation);
			return true;
		}
	}

	return false;
}

bool _there_was_an_error(ready_count_t* ready_count) {
	return atomic_load(ready_count) == ERROR;
}

bool _waiting_for(PollEntry* entry, Operation operation) {
	return entry->poll_pointer->events & _operation_map[operation];
}

bool _tell_that_ready_for(PollEntry* entry, Operation operation) {
	return entry->poll_pointer->revents |= _operation_map[operation];
}

bool _timeout_elapsed(size_t start, size_t timeout) {
	return (now() - start) > timeout;
}

int _install_poll_signal_handler(struct sigaction* old_action) {
	struct sigaction signal_action;

	// Set our function as the signal handling function
	signal_action.sa_handler = _poll_signal_handler;

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// Do not set SA_RESTART! This is precisely the point of all of this!
	// By NOT (NOT NOT NOT) setting SA_RESTART, any interrupted syscall will
	// NOT (NOT NOT NOT) restart automatically. Rather, it will fail with exit
	// code  EINTR, which is precisely what we want.
	signal_action.sa_flags = 0;

	// Don't block any other signals during our exception handling
	sigemptyset(&signal_action.sa_mask);

	if (sigaction(POLL_SIGNAL, &signal_action, old_action) == ERROR) {
		print_error("Error setting signal handler for poll");
		return ERROR;
	}

	return SUCCESS;
}

int _restore_old_signal_action(struct sigaction* old_action) {
	if (sigaction(POLL_SIGNAL, old_action, NULL) == ERROR) {
		print_error("Error restoring old signal handler for poll");
		return ERROR;
	}

	return SUCCESS;
}

void _poll_signal_handler(int signal_number) {
	assert(signal_number == POLL_SIGNAL);
#ifdef DEBUG
	fprintf(stderr, "Received SIGUSR1 in poll\n");
#endif
}

void _cleanup(Vector* tssx_fds, Vector* other_fds) {
	vector_destroy(tssx_fds);
	vector_destroy(other_fds);
}
