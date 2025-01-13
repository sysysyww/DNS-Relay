#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>

#include "hash_table.h"
#include "ip_address.h"
#include "log.h"

// Global variable declaration
extern char upper_server_address[16];

// function declarations
int forward(char *query, int query_len, int tid, char **response_to_client);
int send_back_to_client(char *response, int response_len,
                        struct sockaddr_in *client_addr, int dns_socket,
                        int tid);
#endif  // COMMUNICATION_H