#define _GNU_SOURCE

#include <assert.h>
#include <dlfcn.h>
#include <pthread.h>
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
	int ready_count = 0;

	// HACK: just translate fds
	//	for (int i = 0; i<nfds; ++i) {
	//		fds[i].fd = bridge_deduce_file_descriptor(&bridge, fds[i].fd);
	//	}

	_partition(&tssx_fds, &other_fds, fds, nfds);

	if (tssx_fds.size == 0) {
		// We are only dealing with normal fds -> simply forward
		ready_count = real_poll(fds, nfds, timeout);
	} else if (other_fds.size == 0) {
		// We are only dealing with tssx connections -> check these without spawning
		// threads
		ready_count = _tssx_poll(&tssx_fds, timeout);
	} else {
		// TODO: Otherwise: we are dealing with peter's wip code ;p
		throw("not implemented");

		ready_count_t threaded_ready_count = 0;

		// new threads
		// should take a struct {vector, timeout, &threaded_ready_count}
		// then just operate on the atomic ready count
		threaded_ready_count += _tssx_poll(&tssx_fds, timeout);
		threaded_ready_count += _other_poll(&other_fds, timeout);

		// join tssx_poll
		// join other_poll
		// if ready_count still 0 => timeout on both (return timeout)
		// else return sum of return values

		ready_count = atomic_load(&threaded_ready_count);
	}

	vector_destroy(&tssx_fds);
	vector_destroy(&other_fds);

	return ready_count;
}

/******************** HELPERS ********************/

const short operation_map[2] = {POLLIN, POLLOUT};

void _partition(Vector* tssx_fds,
								Vector* other_fds,
								pollfd fds[],
								nfds_t number) {
	vector_setup(tssx_fds, 32, sizeof(PollEntry));
	vector_setup(other_fds, 32, sizeof(pollfd));

	// for (nfds_t index = 0; index < number; ++index) {
	// 	if (fds[index].fd >= TSSX_KEY_OFFSET) {
	// 		Session* session = bridge_lookup(&bridge, fds[index].fd);
	// 		assert(session_is_valid(session));
	// 		if (session->connection != NULL) {
	// 			PollEntry entry;
	// 			entry.connection = session->connection;
	// 			entry.poll_pointer = &fds[index];
	// 			vector_push_back(tssx_fds, &entry);
	// 			continue;
	// 		}
	// 	}
	//
	// 	vector_push_back(other_fds, &fds[index]);
	// }
}

int _other_poll(Vector* other_fds, int timeout) {
	int ready_count;
	pollfd* raw = other_fds->data;
	size_t size = other_fds->size;

	ready_count = real_poll(raw, size, timeout);

	pthread_exit(0);
	return ready_count;
}

int _tssx_poll(Vector* tssx_fds, int timeout) {
	size_t start = current_milliseconds();
	int ready_count = 0;

	while (!_timeout_elapsed(start, timeout)) {
		VECTOR_FOR_EACH(tssx_fds, iterator) {
			PollEntry* entry = (PollEntry*)iterator_get(&iterator);
			if (_check_ready(entry, READ) || _check_ready(entry, WRITE)) {
				++ready_count;
			}
		}
		if (ready_count > 0) break;// TODO "condvar is set" ????
	}

	return ready_count;
}

bool _check_ready(PollEntry* entry, Operation operation) {
	if (_waiting_for(entry, operation)) {
		if (_ready_for(entry->connection, operation)) {
			_tell_that_ready_for(entry, operation);
			return true;
		}
	}

	return false;
}

bool _waiting_for(PollEntry* entry, Operation operation) {
	return entry->poll_pointer->events & operation_map[operation];
}

bool _tell_that_ready_for(PollEntry* entry, Operation operation) {
	return entry->poll_pointer->revents |= operation_map[operation];
}

bool _timeout_elapsed(size_t start, size_t timeout) {
	return (now() - start) > timeout;
}
