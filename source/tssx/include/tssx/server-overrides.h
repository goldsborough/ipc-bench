#ifndef SERVER_OVERRIDES_H
#define SERVER_OVERRIDES_H

/******************** DEFINITIONS ********************/

struct Session;

/******************** HELPERS ********************/

int send_segment_id_to_client(int client_socket, struct Session* session);
int setup_tssx(int client_socket);

#endif /* SERVER_OVERRIDES_H */
