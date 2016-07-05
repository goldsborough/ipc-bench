#define _GNU_SOURCE

#include <assert.h>
#include <dlfcn.h>
#include <pthread.h>
#include <sys/time.h>

#include "tssx/bridge.h"
#include "tssx/poll-override.h"
#include "tssx/session.h"
#include "vector/vector.h"

/******************** REAL FUNCTION ********************/

int real_poll(pollfd fds[], nfds_t nfds, int timeout) {
	return ((real_poll_t)dlsym(RTLD_next, "poll"))(fds, nfds, timeout);
}

/******************** COMMON OVERRIDES ********************/

int poll(pollfd fds[], nfds_t number, int timeout) {
	Vector tssx_fds, other_fds;
	ready_count_t ready_count = 0;

	partition(&tssx_fds, &other_fds, fds, number);

	// new threads
	// should take a struct {vector, timeout, &ready_count}
	// then just operate on the atomic ready count
	tssx_poll(&tssx_fds, timeout);
	other_poll(&other_fds, timeout);

	// join tssx_poll
	// join other_poll
	// if ready_count still 0 => timeout on both (return timeout)
	// else return sum of return values

	vector_destroy(&tssx_fds);
	vector_destroy(&other_fds);
}

/******************** HELPERS ********************/

const short operation_map[2] = {POLLIN, POLLOUT};

void partition(Vector* tssx_fds,
							 Vector* other_fds,
							 pollfd fds[],
							 nfds_t number) {
	vector_setup(tssx_fds, 32, sizeof(PollEntry));
	vector_setup(other_fds, 32, sizeof(pollfd));

	for (nfds_t index = 0; index < number; ++index) {
		if (fds[index] < TSSX_KEY_OFFSET) {
			vector_push_back(other_fds, fds[index]);
		} else {
			PollEntry entry = create_entry(&fds[index]);
			vector_push_back(tssx_fd, &entry);
		}
	}
}

PollEntry create_entry(pollfd* poll_pointer) {
	PollEntry entry;

	key_t key = bridge_reverse_lookup(&bridge, poll_pointer->fd);
	Session* session = bridge_lookup(&bridge, key);

	assert(session_is_valid(session));
	assert(session.connection != NULL);

	entry.connection = session.connection;
	entry.poll_pointer = poll_pointer;

	return entry;
}

int other_poll(Vector* other_fds, int timeout) {
	int ready_count;
	pollfd* raw = other_fds->data;
	size_t size = other_fds->size;

	ready_count = real_poll(raw, size, timeout);

	pthread_exit();
}

int tssx_poll(Vector* tssx_fds, size_t timeout) {
	size_t start = now();
	int ready_count = 0;

	while (!timeout_elapsed(start, timeout)) {
		VECTOR_FOR_EACH(tssx_fds, iterator) {
			Entry* entry = (Entry*)iterator_get(&iterator);
			if (check_ready(entry, READ) || check_ready(entry, WRITE)) {
				++ready_count;
			}
		}
		if (ready_count > 0 || condvar is set) break;
	}

	exit ready_count
}

bool check_ready(PollEntry* entry, Operation operation) {
	if (waiting_for(entry, operation)) {
		if (ready_for(entry->connection, operation)) {
			tell_that_ready_for(entry, operation);
			return true;
		}
	}

	return false;
}

bool waiting_for(PollEntry* entry, Operation operation) {
	return entry->poll_pointer->events & operation_map[operation];
}

bool tell_that_ready_for(PollEntry* entry, Operation operation) {
	return entry->poll_pointer->revents |= operation_map[operation];
}

size_t now() {
	size_t milliseconds;
	timeval current_time;

	if (gettimeofday(&current_time, NULL) == -1) {
		throw("Error getting time");
	}

	milliseconds = current_time.tv_sec * 1000;
	milliseconds += current_time.tv_usec / 1000;

	return milliseconds;
}

bool timeout_elapsed(size_t start, size_t timeout) {
	return (now() - start) > timeout;
}
