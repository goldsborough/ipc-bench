#ifndef CONNECTION_H
#define CONNECTION_H

#include <stdatomic.h>

/******************** DEFINITIONS ********************/

#define CONNECTION_INITIALIZER \
	{ -1, NULL, NULL, NULL }

struct ConnectionOptions;

/******************** STRUCTURES ********************/

struct Buffer;

typedef atomic_uint_fast16_t atomic_count_t;

typedef struct Connection {
	// The ID of the shared memory
	int segment_id;

	// Reference count of open connections
	atomic_count_t* open_count;

	// The cast shared memory for the server buffer
	struct Buffer* server_buffer;

	// The cast shared memory for the client buffer
	struct Buffer* client_buffer;

} Connection;

/******************** INTERFACE ********************/

Connection* create_connection(const struct ConnectionOptions* options);
Connection* setup_connection(int segment_id,
														 const struct ConnectionOptions* options);

void disconnect(Connection* connection);
void connection_add_user(Connection* connection);

/******************** PRIVATE ********************/

void _create_server_buffer(Connection* connection,
													 void* shared_memory,
													 const struct ConnectionOptions* options);
void _create_client_buffer(Connection* connection,
													 void* shared_memory,
													 const struct ConnectionOptions* options);

void _init_open_count(Connection* connection, void* shared_memory);
void _init_and_increment_open_count(Connection* connection,
																		void* shared_memory);

/******************** UTILITY ********************/

void _detach_connection(Connection* connection);
void _destroy_connection(Connection* connection);

void* _segment_start(Connection* connection);
int _connection_segment_size(Connection* connection);

#endif /* CONNECTION_H */
