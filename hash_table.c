#include <arpa/inet.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ip_address.h"

#define TABLE_SIZE 500

typedef struct hash_table_item_struct *hash_table_item;
typedef struct hash_table_item_struct *hash_table;

struct hash_table_item_struct {
  char *domain_name;
  struct ip_list_node_struct *ip_addresses;
  int version;
  time_t expiration_time;
  struct hash_table_item_struct *next;
};

// BKDR哈希函数
unsigned int BKDRHash(char *str, int version) {
  unsigned int seed = 131;  // 31 131 1313 13131 131313 etc..
  unsigned int hash = 0;
  while (*str) {
    hash = hash * seed + (*str++);
  }
  return (hash & 0x7FFFFFFF + version) % TABLE_SIZE;
}

hash_table_item create_new_hash_node(char *domain, ip_list list,
                                     time_t expiration_time, int version) {
  hash_table_item newNode =
      (hash_table_item)malloc(sizeof(struct hash_table_item_struct));
  if (!newNode) {
    perror("malloc");
    return NULL;
  }

  newNode->domain_name = strdup(domain);
  if (!newNode->domain_name) {
    perror("strdup");
    free(newNode);
    return NULL;
  }

  newNode->ip_addresses = list;
  newNode->expiration_time = expiration_time;
  newNode->version = version;
  newNode->next = NULL;
  return newNode;
}

// 在哈希表中插入一个元素
int insert(unsigned int index, hash_table_item newNode, hash_table *table) {
  if (table[index] == NULL) {
    table[index] = newNode;
  } else {
    hash_table_item current_ptr = table[index];
    hash_table_item pre_ptr = NULL;
    while (current_ptr != NULL) {
      if (current_ptr->domain_name == newNode->domain_name &&
          current_ptr->version == newNode->version &&
          current_ptr->expiration_time > time(NULL)) {
        return -1;
      }
      pre_ptr = current_ptr;
      current_ptr = current_ptr->next;
    }
    pre_ptr->next = newNode;
  }
  return 0;
}

// 从哈希表中查找一个元素
ip_list find(char *domain, hash_table *table, int version,
             time_t *expiration_time) {
  unsigned int index = BKDRHash(domain, version);

  hash_table_item item = table[index];

  hash_table_item last_match = NULL;
  while (item != NULL) {
    if (strcmp(item->domain_name, domain) == 0) {
      if (item->version == version) {
        last_match = item;
      }
    }
    item = item->next;
  }
  // printIpList(last_match->ip_addresses);
  time_t current_time = time(NULL);
  if (last_match != NULL) {
    if (current_time < last_match->expiration_time) {
      *expiration_time = last_match->expiration_time;
      return last_match->ip_addresses;
    }
  }
  return NULL;
}

hash_table_item find_or_create_item(char *domain, hash_table *table,
                                    time_t expiration_time, int version) {
  unsigned int index = BKDRHash(domain, version);
  hash_table_item item = table[index];

  while (item) {
    if (strcmp(item->domain_name, domain) == 0 && item->version == version) {
      return item;  // If the domain exists in the table, return it
    }
    item = item->next;
  }

  // If the item does not exist, insert it into the table
  ip_list list =
      NULL;  // Initialize an empty list here, as the list will be filled later
  hash_table_item newNode =
      create_new_hash_node(domain, list, expiration_time, version);
  insert(index, newNode, table);

  return newNode;
}

void load_hash(char *file_name, hash_table *table) {
  FILE *file = fopen(file_name, "r");
  if (file == NULL) {
    perror("fopen");
    return;
  }

  char ip_str[INET6_ADDRSTRLEN];
  char domain[256];
  time_t expiration_time;

  while (fscanf(file, "%s %s %ld", ip_str, domain, &expiration_time) == 3) {
    hash_table_item item = find_or_create_item(domain, table, expiration_time,
                                               strlen(ip_str) > 15 ? 6 : 4);

    item->ip_addresses = add_to_ip_list(item->ip_addresses, ip_str);
  }

  fclose(file);
}