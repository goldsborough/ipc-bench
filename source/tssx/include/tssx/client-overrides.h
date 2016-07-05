#ifndef CLIENT_OVERRIDES_H
#define CLIENT_OVERRIDES_H

/******************** DEFINITIONS ********************/

struct Session;
struct sockaddr;

/******************** FORWARD DECLARATIONS ********************/

int read_segment_id_from_server(int client_socket);
int setup_tssx(struct Session* session, const struct sockaddr* address);

#endif /* CLIENT_OVERRIDES_H */
