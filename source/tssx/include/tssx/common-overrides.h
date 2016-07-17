#ifndef COMMON_OVERRIDES_H
#define COMMON_OVERRIDES_H

#include <assert.h>
#include <dlfcn.h>
#include <stdbool.h>

#include "common/utility.h"
#include "tssx/bridge.h"
#include "tssx/buffer.h"
#include "tssx/connection-options.h"
#include "tssx/connection.h"
#include "tssx/selective.h"
#include "tssx/session.h"
#include "tssx/shared-memory.h"

#include "tssx/socket-overrides.h"

/******************** DEFINITIONS ********************/

#define ERROR -1
#define SUCCESS 0
#define SERVER_BUFFER 0
#define CLIENT_BUFFER 1

struct Connection;
struct Buffer;

typedef int (*real_fcntl_t)(int, int, ...);
typedef pid_t (*real_fork_t)(void);

/******************** REAL FUNCTIONS ********************/

int real_fcntl_set_flags(int fd, int command, int flag);
int real_fcntl_get_flags(int fd, int command);

pid_t real_fork(void);

/******************** COMMON OVERRIDES ********************/

pid_t fork(void);

/******************** INTERFACE ********************/

ssize_t connection_write(int key,
												 const void* source,
												 size_t requested_bytes,
												 int which_buffer);
ssize_t connection_read(int key,
												void* destination,
												size_t requested_bytes,
												int which_buffer);

int socket_is_stream_and_domain(int domain, int type);

/******************** HELPERS ********************/

// Declarations only (defintions in server/client overrides)
void set_non_blocking(Connection* connection, bool non_blocking);
bool get_non_blocking(Connection* connection);

int fcntl_set(int fd, int command, int flags);
int fcntl_get(int fd, int command);

struct Buffer* get_buffer(struct Connection* connection, int which_buffer);

#endif /* COMMON_OVERRIDES_H */
