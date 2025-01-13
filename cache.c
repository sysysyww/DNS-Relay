#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>

#include "hash_table.h"
#include "ip_address.h"
#include "log.h"

ip_list check_cache(char *domain_name, int version, hash_table *table,
                    time_t *expiration_time) {
  ip_list result_list = find(domain_name, table, version, expiration_time);
  if (result_list != NULL) {
    return result_list;
  }
  return NULL;
}
