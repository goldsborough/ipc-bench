#ifndef CLIENT_OVERRIDES_H
#define CLIENT_OVERRIDES_H

#include "tssx/definitions.h"

/******************** DEFINITIONS ********************/

struct Session;
struct sockaddr;

/******************** FORWARD DECLARATIONS ********************/

int read_segment_id_from_server(int client_socket);
int setup_tssx(int fd);

#endif /* CLIENT_OVERRIDES_H */
