#ifndef ENCAPSULATION_H
#define ENCAPSULATION_H

#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "dns_struct.h"
#include "hash_table.h"
#include "ip_address.h"

response *generate_response(dns_header_t *header, char *query, ip_list ipList,
                            time_t time1);
response *generate_errorresponse(dns_header_t *header, char *query,
                                 ip_list ipList, time_t time1);
unsigned char *response_to_buffer(response *res, int *length);

#endif  // ENCAPSULATION_H
