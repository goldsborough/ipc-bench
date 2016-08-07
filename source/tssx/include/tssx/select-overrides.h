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
												size_t highest_fd);

int _read_poll_entries(DescriptorSets *sets,
											 struct pollfd *poll_entries,
											 size_t population_count);

int _select_on_tssx_only(DescriptorSets *sets,
												 size_t tssx_count,
												 size_t lowest_fd,
												 size_t highest_fd,
												 struct timeval *timeout);

int _select_on_tssx_only_fast_path(DescriptorSets *sets,
																	 size_t fd,
																	 struct timeval *timeout);

void _count_tssx_sockets(size_t highest_fd,
												 const DescriptorSets *sets,
												 size_t *lowest_fd,
												 size_t *normal_count,
												 size_t *tssx_count);

bool _is_in_any_set(int fd, const DescriptorSets *sets);

void _select_operation(const Session *session,
											 Operation operation,
											 fd_set *set,
											 int fd);

void _copy_all_sets(DescriptorSets *destination, const DescriptorSets *source);
void _copy_set(fd_set *destination, const fd_set *source);

void _clear_all_sets(DescriptorSets *sets);
bool _fd_is_set(int fd, const fd_set *set);

bool _select_timeout_elapsed(size_t start, int timeout);

void _clear_set(fd_set *set);

#endif /* SELECT_OVERRIDES_H */
