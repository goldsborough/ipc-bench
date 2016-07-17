#ifndef REVERSE_MAP_H
#define REVERSE_MAP_H

#include <sys/select.h>
#include <sys/types.h>

#include "definitions.h"

/*
* The point of this is to hide the implementation of the underlying
* reverse-map container via functions. If we just used the array *currently*
* used below as the container, then we'd have to do array indexing. However,
* we might decide at a later point that using some other data structure, such
* as a (hash) set maybe, would be better. Then we'd have to recode all our
* accesses. This way, we can just change the implementations of the methods
* and no external code using this interface must be touched (profit)
*/

/******************** DEFINITIONS ********************/

#define REVERSE_MAP_ERROR 0
#define REVERSE_MAP_SUCCESS 1

#define REVERSE_MAP_SIZE FD_SETSIZE
#define INVALID_KEY 0

typedef key_t ReverseMap[REVERSE_MAP_SIZE];

/******************** INTERFACE ********************/

int reverse_map_setup(ReverseMap* reverse);
int reverse_map_destroy(ReverseMap* reverse);

key_t reverse_map_lookup(ReverseMap* reverse, int socket_fd);
int reverse_map_has_entry_for(ReverseMap* reverse, int socket_fd);

int reverse_map_erase(ReverseMap* reverse, int socket_fd);
int reverse_map_insert(ReverseMap* reverse, int socket_fd, int fd);

ssize_t reverse_map_size(const ReverseMap* map);
int reverse_map_is_empty(const ReverseMap* map);

/******************** PRIVATE ********************/

#endif /* REVERSE_MAP_H */
