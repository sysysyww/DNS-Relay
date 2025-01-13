#ifndef CACHE_H
#define CACHE_H

#include <netinet/in.h>

#include "hash_table.h"
#include "ip_address.h"

ip_list check_cache(char *domain_name, int version, hash_table *table, time_t *expiration_time);
#endif // CACHE_H
