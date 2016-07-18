#ifndef SELECT_OVERRIDES_H
#define SELECT_OVERRIDES_H

#include <stdbool.h>
#include <stddef.h>
#include <sys/select.h>

#include "tssx/definitions.h"

/******************** DEFINITIONS ********************/

typedef int (*real_select_t)(
		int, fd_set *, fd_set *, fd_set *, struct timeval *);

struct pollfd;

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
										 DescriptorSets *sets,
										 size_t population_count,
										 struct timeval *timeout);

struct pollfd *_allocate_poll_entries(size_t population_count);

struct pollfd *_setup_poll_entries(size_t population_count,
																	 const DescriptorSets *sets,
																	 size_t highest_fd);
void _fill_poll_entries(struct pollfd *poll_entries,
												const DescriptorSets *sets,
												size_t hightest_fd);

int _read_poll_entries(DescriptorSets *sets,
											 struct pollfd *poll_entries,
											 size_t population_count);

bool _at_least_one_socket_uses_tssx(size_t highest_fd,
																		const DescriptorSets *sets,
																		size_t *population_count);

bool _is_in_any_set(int fd, const DescriptorSets *sets);

void _clear_all_sets(DescriptorSets *sets);
bool _fd_is_set(int fd, const fd_set *set);

#endif /* SELECT_OVERRIDES_H */
