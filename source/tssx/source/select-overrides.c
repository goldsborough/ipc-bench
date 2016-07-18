#define _GNU_SOURCE

#include <dlfcn.h>
#include <poll.h>
#include <stdlib.h>

#include "common/utility.h"
#include "tssx/poll-overrides.h"

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
	size_t population_count;
	DescriptorSets sets = {readfds, writefds, errorfds};

	puts("Inside select\n");
	
	if (_at_least_one_socket_uses_tssx(nfds, sets, &population_count)) {
	  puts("Forwarding select to poll\n");
		return _forward_to_poll(nfds, &sets, population_count, timeout);
	} else {
	  puts("Calling real_select with only non-tssx sockets\n");
		return real_select(nfds, readfds, writefds, exceptfds, timeout);
	}
}

int _forward_to_poll(size_t highest_fd,
										 DescriptorSets sets,
										 size_t population_count,
										 struct timeval* timeout) {
	pollfd* poll_entries;
	size_t number_of_events;
	int milliseconds;

	if ((poll_entries = _setup_poll_entries(population_count, &sets)) == NULL) {
		return ERROR;
	}
	milliseconds = timeout_to_milliseconds(timeout);

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

pollfd* _setup_poll_entries(size_t population_count,
														const DescriptorSets* sets) {
	pollfd* poll_entries;

	if ((poll_entries = _allocate_poll_entries(population_count)) == NULL) {
		return NULL;
	}

	_fill_poll_entries(poll_entries, sets);

	return poll_entries;
}

int _read_poll_entries(DescriptorSets* sets,
											 pollfd* poll_entries,
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
}

pollfd* _allocate_poll_entries(size_t population_count) {
	pollfd* poll_entries;
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

void _fill_poll_entries(pollfd* poll_entries, const DescriptorSets* sets) {
	size_t poll_index = 0;

	for (size_t fd = 0; fd < hightest_fd; ++fd) {
		if (_is_in_any_set(fd, sets)) {
			poll_entries[poll_index].fd = fd;

			if (FD_ISSET(fd, sets.readfds)) {
			  printf("Polling read event for %d\n", fd);
				poll_entries[poll_index].events |= POLLIN;
			}
			if (FD_ISSET(fd, sets.writefds)) {
			  printf("Polling write event for %d\n", fd);
				poll_entries[poll_index].events |= POLLOUT;
			}
			if (FD_ISSET(fd, sets.errorfds)) {
			  printf("Polling error event for %d\n", fd);
				poll_entries[poll_index].events |= POLLERR;
			}

			++poll_index;
		}
	}
}

bool _at_least_one_socket_uses_tssx(size_t highest_fd,
																		const DescriptorSets* sets,
																		size_t* population_count) {
	*population_count = 0;

	for (size_t fd = 0; fd < highest_fd; ++fd) {
		// clang-format off
		if (_is_in_any_set(fd, sets)) {
      ++(*population_count);
			if (bridge_has_connection(bridge, fd)) return true;
		}
		// clang-format on
	}

	return false;
}

void _clear_all_sets(DescriptorSets* sets) {
	FD_ZERO(sets->readfds);
	FD_ZERO(sets->writefds);
	FD_ZERO(sets->errorfds);
}

bool _is_in_any_set(int fd, const DescriptorSets* sets) {
	if (FD_ISSET(fd, sets->readfds) return true;
  if (FD_ISSET(fd, sets->writefds) return true;
  if (FD_ISSET(fd, sets->errorfds) return true;

  return false;
}
