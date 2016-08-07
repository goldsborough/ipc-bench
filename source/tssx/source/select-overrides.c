#define _GNU_SOURCE

#include <assert.h>
#include <dlfcn.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/signal.h>

#include "common/utility.h"
#include "tssx/bridge.h"
#include "tssx/poll-overrides.h"
#include "tssx/select-overrides.h"

/******************** REAL FUNCTION ********************/

int real_select(int nfds,
								fd_set* readfds,
								fd_set* writefds,
								fd_set* errorfds,
								struct timeval* timeout) {
	// clang-format off
	return ((real_select_t)dlsym(RTLD_NEXT, "select"))
            (nfds, readfds, writefds, errorfds, timeout);
	// clang-format on
}

/******************** OVERRIDES ********************/

int select(int nfds,
					 fd_set* readfds,
					 fd_set* writefds,
					 fd_set* errorfds,
					 struct timeval* timeout) {
	DescriptorSets sets = {readfds, writefds, errorfds};
	size_t tssx_count, normal_count, lowest_fd;
	_count_tssx_sockets(nfds, &sets, &lowest_fd, &normal_count, &tssx_count);

	if (normal_count == 0) {
		return _select_on_tssx_only(&sets, tssx_count, lowest_fd, nfds, timeout);
	} else if (tssx_count == 0) {
		return real_select(nfds, readfds, writefds, errorfds, timeout);
	} else {
		return _forward_to_poll(nfds, &sets, tssx_count, timeout);
	}
}

int _forward_to_poll(size_t highest_fd,
										 DescriptorSets* sets,
										 size_t population_count,
										 struct timeval* timeout) {
	struct pollfd* poll_entries;
	size_t number_of_events;
	int milliseconds;

	poll_entries = _setup_poll_entries(population_count, sets, highest_fd);
	if (poll_entries == NULL) {
		return ERROR;
	}

	milliseconds = timeout ? timeval_to_milliseconds(timeout) : BLOCK_FOREVER;

	// The actual forwarding call
	number_of_events = poll(poll_entries, population_count, milliseconds);

	if (number_of_events == ERROR) {
		free(poll_entries);
		return ERROR;
	}

	if (_read_poll_entries(sets, poll_entries, population_count) == ERROR) {
		return ERROR;
	}

	return number_of_events;
}

struct pollfd* _setup_poll_entries(size_t population_count,
																	 const DescriptorSets* sets,
																	 size_t highest_fd) {
	struct pollfd* poll_entries;

	if ((poll_entries = _allocate_poll_entries(population_count)) == NULL) {
		return NULL;
	}

	_fill_poll_entries(poll_entries, sets, highest_fd);

	return poll_entries;
}

int _read_poll_entries(DescriptorSets* sets,
											 struct pollfd* poll_entries,
											 size_t population_count) {
	// First unset all, then just repopulate
	_clear_all_sets(sets);

	for (size_t index = 0; index < population_count; ++index) {
		int fd = poll_entries[index].fd;

		if (poll_entries[index].revents & POLLNVAL) {
			free(poll_entries);
			assert(false);
			return ERROR;
		}

		if (poll_entries[index].revents & POLLIN) {
			FD_SET(fd, sets->readfds);
		}
		if (poll_entries[index].revents & POLLOUT) {
			FD_SET(fd, sets->writefds);
		}
		if (poll_entries[index].revents & POLLERR) {
			FD_SET(fd, sets->errorfds);
		}
	}

	free(poll_entries);

	return SUCCESS;
}

struct pollfd* _allocate_poll_entries(size_t population_count) {
	struct pollfd* poll_entries;
	size_t poll_entries_length;

	poll_entries_length = sizeof *poll_entries * population_count;
	poll_entries = malloc(poll_entries_length);

	if (poll_entries == NULL) {
		perror("Error allocating memory for poll entries");
		return NULL;
	}

	memset(poll_entries, 0, poll_entries_length);

	return poll_entries;
}

void _fill_poll_entries(struct pollfd* poll_entries,
												const DescriptorSets* sets,
												size_t highest_fd) {
	size_t poll_index = 0;

	for (size_t fd = 0; fd < highest_fd; ++fd) {
		if (!_is_in_any_set(fd, sets)) continue;

		poll_entries[poll_index].fd = fd;

		if (_fd_is_set(fd, sets->readfds)) {
			poll_entries[poll_index].events |= POLLIN;
		}
		if (_fd_is_set(fd, sets->writefds)) {
			poll_entries[poll_index].events |= POLLOUT;
		}
		if (_fd_is_set(fd, sets->errorfds)) {
			poll_entries[poll_index].events |= POLLERR;
		}

		++poll_index;
	}
}

int _select_on_tssx_only(DescriptorSets* sets,
												 size_t tssx_count,
												 size_t lowest_fd,
												 size_t highest_fd,
												 struct timeval* timeout) {
	if (tssx_count == 1) {
		assert(lowest_fd + 1 == highest_fd);
		return _select_on_tssx_only_fast_path(sets, lowest_fd, timeout);
	}

	size_t start = current_milliseconds();
	int ready_count = 0;
	int milliseconds = timeout ? timeval_to_milliseconds(timeout) : BLOCK_FOREVER;

	fd_set readfds, writefds, errorfds;
	DescriptorSets original = {&readfds, &writefds, &errorfds};
	_copy_all_sets(original, sets);
	_clear_all_sets(sets);

	// Do-while for the case of non-blocking
	// so that we do at least one iteration
	do {
		for (size_t fd = lowest_fd; fd < highest_fd; ++fd) {
			Session* session = bridge_lookup(&bridge, fd);
			assert(session != NULL);
			bool ready_for_reading = false;
			bool ready_for_writing = false;

			if (_fd_is_set(fd, original.readfds)) {
				ready_for_reading = _ready_for(session->connection, READ);
				if (ready_for_reading) FD_SET(fd, sets->readfds);
			}
			if (_fd_is_set(fd, original.writefds)) {
				ready_for_writing = _ready_for(session->connection, WRITE);
				if (ready_for_writing) FD_SET(fd, sets->writefds);
			}

			if (ready_for_reading || ready_for_writing) ready_count++;
		}
		if (ready_count > 0) break;
	} while (!_select_timeout_elapsed(start, milliseconds));

	return ready_count;
}

int _select_on_tssx_only_fast_path(DescriptorSets* sets,
																	 size_t fd,
																	 struct timeval* timeout) {
	bool select_read = _fd_is_set(fd, sets->readfds);
	bool select_write = _fd_is_set(fd, sets->writefds);
	assert(select_read || select_write);

	_clear_all_sets(sets);

	size_t start = current_milliseconds();
	int milliseconds = timeout ? timeval_to_milliseconds(timeout) : BLOCK_FOREVER;

	Session* session = bridge_lookup(&bridge, fd);

	bool ready = false;
	if (select_read && select_write) {
		do {
			if (_select_operation(session, WRITE, sets->writefds)) ready = true;
			if (_select_operation(session, READ, sets->readfds)) ready = true;
		} while (!ready && !_select_timeout_elapsed(start, milliseconds));
	} else if (select_read) {
		do {
			if (_select_operation(session, READ, sets->readfds, fd)) ready = true;
		} while (!ready && !_select_timeout_elapsed(start, milliseconds));
	} else {
		do {
			if (_select_operation(session, WRITE, sets->writefds, fd)) ready = true;
		} while (!ready && !_select_timeout_elapsed(start, milliseconds));
	}

	return ready;
}

void _select_operation(const Session* session,
											 Operation operation,
											 fd_set* set,
											 int fd) {
	if (_ready_for(session->connection, operation)) {
		FD_SET(fd, set);
	}
}

void _count_tssx_sockets(size_t highest_fd,
												 const DescriptorSets* sets,
												 size_t* lowest_fd,
												 size_t* normal_count,
												 size_t* tssx_count) {
	*normal_count = 0;
	*tssx_count = 0;
	*lowest_fd = highest_fd;

	for (size_t fd = 0; fd < highest_fd; ++fd) {
		if (_is_in_any_set(fd, sets)) {
			if (fd < *lowest_fd) {
				*lowest_fd = fd;
			}
			if (bridge_has_connection(&bridge, fd)) {
				++(*tssx_count);
			} else {
				++(*normal_count);
			}
		}
	}
}

void _copy_all_sets(DescriptorSets* destination, const DescriptorSets* source) {
	_copy_set(destination->readfs, source->readfds);
	_copy_set(destination->writefds, source->writefds);
	_copy_set(destination->errorfds, source->errorfds);
}

void _copy_set(fd_set* destination, const fd_set* source) {
	if (source == NULL) {
		FD_ZERO(destination);
	} else {
		FD_COPY(source, destination);
	}
}

void _clear_all_sets(DescriptorSets* sets) {
	_clear_set(sets->readfs);
	_clear_set(sets->write);
	_clear_set(sets->errorfds);
}

bool _is_in_any_set(int fd, const DescriptorSets* sets) {
	if (_fd_is_set(fd, sets->readfds)) return true;
	if (_fd_is_set(fd, sets->writefds)) return true;
	if (_fd_is_set(fd, sets->errorfds)) return true;

	return false;
}

bool _fd_is_set(int fd, const fd_set* set) {
	return set != NULL && FD_ISSET(fd, set);
}

bool _select_timeout_elapsed(size_t start, int timeout) {
	if (timeout == BLOCK_FOREVER) return false;
	if (timeout == DONT_BLOCK) return true;

	return (current_milliseconds() - start) > timeout;
}

void _clear_set(fd_set* set) {
	if (set != NULL) FD_ZERO(set);
}
