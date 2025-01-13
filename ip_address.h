#ifndef IP_ADDRESS_H
#define IP_ADDRESS_H

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

typedef struct ip_address_struct *ip_address;
typedef struct ip_list_node_struct *ip_list;
typedef struct ip_list_node_struct *ip_list_node;

struct ip_address_struct {
  socklen_t len;                 // 地址的长度
  struct sockaddr_storage addr;  // 地址
};

struct ip_list_node_struct {
  struct ip_address_struct *ip_addr;
  struct ip_list_node_struct *next;
};
typedef struct address_struct *address;

struct address_struct {
  char *ip;
  int port;
};

address convertIPAddress(struct sockaddr_in addr);
ip_list create_ip_list_node(char *ip_str);
ip_list add_to_ip_list(ip_list head, char *ip_str);
int is_address_zero(struct ip_address_struct *ip_struct);
char *ip_to_string(void *ip_addr, int addr_type);
void printIpList(ip_list list);

#endif  // IP_ADDRESS_H
