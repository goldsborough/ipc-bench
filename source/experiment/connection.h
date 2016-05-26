#ifndef CONNECTION_H
#define CONNECTION_H

struct Connection {
	// The ID of the shared memory
	int segment_id;

	// The cast shared memory for the server buffer
	struct Buffer* server_buffer;

	// The cast shared memory for the client buffer
	struct Buffer* client_buffer;
};

void setup_connection(struct Connection* connection, int buffer_size);

void create_server_buffer(struct Connection* connection,
													void* shared_memory,
													int buffer_size);
void create_client_buffer(struct Connection* connection,
													void* shared_memory,
													int buffer_size);

#endif /* CONNECTION_H */
