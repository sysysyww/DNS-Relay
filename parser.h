// parser.h
#pragma once

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "dns_struct.h"
#include "hash_table.h"
#include "ip_address.h"

// function declarations
int recv_parser(char *buffer, char *domain, struct dns_header *hdr);
time_t resolve_upper_response(char *response, int header_and_query_length,
                              ip_list *list);
