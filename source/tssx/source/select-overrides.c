#define _GNU_SOURCE

#include <assert.h>
#include <dlfcn.h>
#include <poll.h>
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
	size_t population_count;
	DescriptorSets sets = {readfds, writefds, errorfds};

	puts("Inside select\n");

	if (_at_least_one_socket_uses_tssx(nfds, &sets, &population_count)) {
		puts("Forwarding select to poll\n");
		return _forward_to_poll(nfds, &sets, population_count, timeout);
	} else {
		puts("Calling real_select with only non-tssx sockets\n");
		return real_select(nfds, readfds, writefds, errorfds, timeout);
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

	printf("Blocking for %d milliseconds\n", milliseconds);

	puts("Calling poll now\n");

	// The actual forwarding call
	number_of_events = poll(poll_entries, population_count, milliseconds);

	if (number_of_events == ERROR) {
		free(poll_entries);
		return ERROR;
	}

	puts("Reading poll entries\n");

	if (_read_poll_entries(sets, poll_entries, population_count) == ERROR) {
		return ERROR;
	}

	printf("There were %lu\n events\n", number_of_events);

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

		printf("Reading events for %d\n", fd);

		if (poll_entries[index].revents & POLLIN) {
			printf("Detected POLLIN event for %d\n", fd);
			FD_SET(fd, sets->readfds);
		}
		if (poll_entries[index].revents & POLLOUT) {
			printf("Detected POLLOUT event for %d\n", fd);
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

	printf("Allocated %lu bytes for %lu poll entries\n",
				 poll_entries_length,
				 population_count);

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

	printf("The highest fd is %lu\n", highest_fd);

	for (size_t fd = 0; fd < highest_fd; ++fd) {
		printf("Checking fd %lu\n", fd);
		if (_is_in_any_set(fd, sets)) {
			poll_entries[poll_index].fd = fd;

			printf("About to check read events for %lu\n", fd);
			if (_fd_is_set(fd, sets->readfds)) {
				printf("Polling read event for %lu\n", fd);
				printf("Previous flags %d\n", poll_entries[poll_index].events);
				printf("Poll index is %lu\n", poll_index);
				poll_entries[poll_index].events |= POLLIN;
				printf("New flags %d\n", poll_entries[poll_index].events);
			}
			printf("About to check write events for %lu\n", fd);
			if (_fd_is_set(fd, sets->writefds)) {
				printf("Polling write event for %lu\n", fd);
				poll_entries[poll_index].events |= POLLOUT;
			}
			printf("About to check error events for %lu\n", fd);
			if (_fd_is_set(fd, sets->errorfds)) {
				printf("Polling error event for %lu\n", fd);
				poll_entries[poll_index].events |= POLLERR;
			}

			++poll_index;
		}
	}

	puts("Done filling poll entries\n");
}

bool _at_least_one_socket_uses_tssx(size_t highest_fd,
																		const DescriptorSets* sets,
																		size_t* population_count) {
	*population_count = 0;

	for (size_t fd = 0; fd < highest_fd; ++fd) {
		// clang-format off
		if (_is_in_any_set(fd, sets)) {
      ++(*population_count);
			if (bridge_has_connection(&bridge, fd)) return true;
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
	if (_fd_is_set(fd, sets->readfds)) return true;
	if (_fd_is_set(fd, sets->writefds)) return true;
	if (_fd_is_set(fd, sets->errorfds)) return true;

	return false;
}

bool _fd_is_set(int fd, const fd_set* set) {
	return set != NULL && FD_ISSET(fd, set);
}
