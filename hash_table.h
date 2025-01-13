#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "ip_address.h"

#define TABLE_SIZE 500

typedef struct hash_table_item_struct *hash_table_item;
typedef struct hash_table_item_struct *hash_table;

struct hash_table_item_struct
{
    char *domain_name;
    struct ip_list_node_struct *ip_addresses;
    int version;
    time_t expiration_time;
    struct hash_table_item_struct *next;
};

unsigned int BKDRHash(char *str, int version);
hash_table_item create_new_hash_node(char *domain, ip_list list, time_t expiration_time, int version);
int insert(unsigned int index, hash_table_item newNode, hash_table *table);
ip_list find(char *domain, hash_table *table, int version, time_t *expiration_time);
hash_table_item find_or_create_item(char *domain, hash_table *table, time_t expiration_time, int version);
void load_hash(char *file_name, hash_table *table);

#endif // HASH_TABLE_H
