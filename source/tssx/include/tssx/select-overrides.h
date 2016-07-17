#ifndef SOCKET_OVERRIDES_H
#define SOCKET_OVERRIDES_H

#include <stdbool.h>
#include <stddef.h>
#include <sys/select.h>

#include "tssx/definitions.h"

/******************** DEFINITIONS ********************/

typedef int (*real_select_t)(
		int, fd_set *, fd_set *, fd_set *, struct timeval *);

typedef struct DescriptorSets {
	fd_set *readfds;
	fd_set *writefds;
	fd_set *errorfds;
} DescriptorSets;

/******************** REAL FUNCTIONS ********************/

int real_select(int nfds,
								fd_set *readfds,
								fd_set *writefds,
								fd_set *errorfds,
								struct timeval *timeout);

/******************** OVERRIDES ********************/

int select(int nfds,
					 fd_set *readfds,
					 fd_set *writefds,
					 fd_set *errorfds,
					 struct timeval *timeout);

/******************** HELPERS ********************/

int _forward_to_poll(size_t highest_fd,
										 const DescriptorSets *sets,
										 size_t *population_count);

pollfd *_allocate_poll_entries(size_t population_count);

pollfd *_setup_poll_entries(size_t population_count,
														const DescriptorSets *sets);
void _fill_poll_entries(pollfd *poll_entries, const DescriptorSets *sets);

void _read_poll_entries(pollfd *poll_entries, DescriptorSets *sets);

bool _at_least_one_socket_uses_tssx(size_t highest_fd,
																		const DescriptorSets *sets,
																		size_t *population_count);

bool _is_in_any_set(int fd, const DescriptorSets *sets);

#endif /* SOCKET_OVERRIDES_H */
