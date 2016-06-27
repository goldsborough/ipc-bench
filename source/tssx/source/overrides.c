#define _GNU_SOURCE

#include <assert.h>
#include <dlfcn.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "bitset/bitset.h"
#include "tssx/buffer.h"
#include "tssx/overrides.h"

Bridge bridge = BRIDGE_INITIALIZER;

// RTDL_NEXT = look in the symbol table of the *next* object file after this one
ssize_t real_write(int fd, const void* data, size_t size) {
	return ((real_write_t)dlsym(RTLD_NEXT, "write"))(fd, data, size);
}

ssize_t real_read(int fd, void* data, size_t size) {
	return ((real_read_t)dlsym(RTLD_NEXT, "read"))(fd, data, size);
}

int real_accept(int fd, sockaddr* address, int* length) {
	return ((real_accept_t)dlsym(RTLD_NEXT, "accept"))(fd, address, length);
}

void real_connect(int fd, const sockaddr* address, int length) {
	((real_connect_t)dlsym(RTLD_NEXT, "connect"))(fd, address, length);
}

int real_close(int fd) {
	return ((real_close_t)dlsym(RTLD_NEXT, "close"))(fd);
}

pid_t real_fork(void) {
	return ((real_fork_t)dlsym(RTLD_NEXT, "fork"))();
}

pid_t fork(void) {
	for (size_t index = 0; index < bridge.table.size; ++index) {
		if (bitset_test(&bridge.occupied, index)) {
			connection_add_user(table_get(&bridge.table, index));
		}
	}

	return real_fork();
}

int connection_write(int key,
										 void* destination,
										 int requested_bytes,
										 int which_buffer) {
	Connection* connection;

	connection = bridge_lookup(&bridge, key);
	assert(connection != NULL);

	// clang-format off
	return buffer_write(
		get_buffer(connection, which_buffer),
		destination,
		requested_bytes
	);
	// clang-format on
}

int connection_read(int key,
										void* source,
										int requested_bytes,
										int which_buffer) {
	Connection* connection;

	connection = bridge_lookup(&bridge, key);
	assert(connection != NULL);

	// clang-format off
	return buffer_read(
		get_buffer(connection, which_buffer),
		source,
		requested_bytes
	);
	// clang-format on
}

Buffer* get_buffer(Connection* connection, int which_buffer) {
	return which_buffer ? connection->client_buffer : connection->server_buffer;
}
