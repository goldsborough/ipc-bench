#ifndef CONNECTION_H
#define CONNECTION_H

#define DEFAULT_BUFFER_SIZE 1000000
#define DEFAULT_TIMEOUT 0

#define DEFAULT_OPTIONS &DEFAULT_OPTIONS_OBJECT

struct Buffer;
typedef struct Buffer Buffer;

typedef struct Connection {
	// The ID of the shared memory
	int segment_id;

	// The cast shared memory for the server buffer
	struct Buffer* server_buffer;

	// The cast shared memory for the client buffer
	struct Buffer* client_buffer;

} Connection;

typedef struct ConnectionOptions {
	int server_buffer_size;
	int server_timeout;

	int client_buffer_size;
	int client_timeout;

} ConnectionOptions;

extern ConnectionOptions DEFAULT_OPTIONS_OBJECT;


void setup_connection(Connection* connection, ConnectionOptions* options);

void server_options_from_socket(ConnectionOptions* options, int socket_fd);
void client_options_from_socket(ConnectionOptions* options, int socket_fd);

void destroy_connection(Connection* connection);
void disconnect(Connection* connection);

void create_server_buffer(Connection* connection,
													void* shared_memory,
													ConnectionOptions* options);
void create_client_buffer(Connection* connection,
													void* shared_memory,
													ConnectionOptions* options);

int options_segment_size(ConnectionOptions* options);
int connection_segment_size(Connection* connection);

#endif /* CONNECTION_H */
