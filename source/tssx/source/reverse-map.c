#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "tssx/reverse-map.h"

int reverse_map_setup(ReverseMap* reverse) {
	assert(reverse != NULL);
	memset(reverse, INVALID_KEY, REVERSE_MAP_SIZE);

	return REVERSE_MAP_SUCCESS;
}

int reverse_map_destroy(ReverseMap* reverse) {
	assert(reverse != NULL);
	memset(reverse, INVALID_KEY, REVERSE_MAP_SIZE);

	return REVERSE_MAP_SUCCESS;
}

key_t reverse_map_lookup(ReverseMap* reverse, int socket_fd) {
	assert(reverse != NULL);
	return (*reverse)[socket_fd];
}

int reverse_map_has_entry_for(ReverseMap* reverse, int socket_fd) {
	assert(reverse != NULL);
	return (*reverse)[socket_fd] != INVALID_KEY;
}

int reverse_map_erase(ReverseMap* reverse, int socket_fd) {
	assert(reverse != NULL);
	assert(reverse_map_has_entry_for(reverse, socket_fd));
	(*reverse)[socket_fd] = INVALID_KEY;

	return REVERSE_MAP_SUCCESS;
}

int reverse_map_insert(ReverseMap* reverse, int socket_fd, key_t key) {
	assert(reverse != NULL);
	assert(!reverse_map_has_entry_for(reverse, socket_fd));
	(*reverse)[socket_fd] = key;

	return REVERSE_MAP_SUCCESS;
}

ssize_t reverse_map_size(const ReverseMap* map) {
	return REVERSE_MAP_SIZE;
}

int reverse_map_is_empty(const ReverseMap* map) {
	return reverse_map_size(map) == 0;
}
