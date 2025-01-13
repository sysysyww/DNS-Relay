// serve.h
#pragma once

#include <arpa/inet.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>

#include "cache.h"
#include "dns_struct.h"
#include "encapsulation.h"
#include "hash_table.h"
#include "ip_address.h"
#include "log.h"

// ... 其他的include和全局变量声明
extern hash_table_item hashTable[TABLE_SIZE];
extern pthread_mutex_t port_53_socket_lock;
extern pthread_mutex_t table_lock[TABLE_SIZE];

// function declaration
void service(void *arg);
