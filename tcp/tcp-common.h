#ifndef IPC_BENCH_TCP_COMMON_H
#define IPC_BENCH_TCP_COMMON_H

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common/common.h"

#define PORT "6969"
#define HOST "localhost"

void print_address(struct addrinfo *address_info);

#endif /* IPC_BENCH_TCP_COMMON_H */
