#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

address convertIPAddress(struct sockaddr_in addr) {
  char ipAddress[INET_ADDRSTRLEN];
  const char *result =
      inet_ntop(AF_INET, &(addr.sin_addr), ipAddress, INET_ADDRSTRLEN);
  if (result != NULL) {
    address result_address = (address)malloc(sizeof(struct address_struct));
    result_address->ip = ipAddress;
    result_address->port = ntohs(addr.sin_port);
    return result_address;
  } else {
    printf("Failed to convert IP address to string.\n");
    return NULL;
  }
}

ip_list create_ip_list_node(char *ip_str) {
  ip_list new_node = (ip_list)malloc(sizeof(struct ip_list_node_struct));
  if (!new_node) {
    perror("malloc");
    return NULL;
  }

  new_node->ip_addr = (ip_address)malloc(sizeof(struct ip_address_struct));
  if (!new_node->ip_addr) {
    perror("malloc");
    free(new_node);
    return NULL;
  }

  struct sockaddr_in *addr4 = (struct sockaddr_in *)&new_node->ip_addr->addr;
  struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)&new_node->ip_addr->addr;

  // Check if it's an IPv4 address
  if (inet_pton(AF_INET, ip_str, &(addr4->sin_addr))) {
    new_node->ip_addr->addr.ss_family = AF_INET;
    new_node->ip_addr->len = 4;
  }
  // Check if it's an IPv6 address
  else if (inet_pton(AF_INET6, ip_str, &(addr6->sin6_addr))) {
    new_node->ip_addr->addr.ss_family = AF_INET6;
    new_node->ip_addr->len = 16;
  } else {
    printf("Invalid IP address\n");
    return NULL;
  }

  new_node->next = NULL;

  return new_node;
}

ip_list add_to_ip_list(ip_list head, char *ip_str) {
  ip_list new_node = create_ip_list_node(ip_str);
  if (!new_node) {
    return NULL;
  }

  if (head == NULL) {
    head = new_node;
    new_node->next = NULL;
    return head;
  }
  ip_list_node currentPtr = head;
  while (currentPtr->next != NULL) {
    currentPtr = currentPtr->next;
  }
  currentPtr->next = new_node;
  new_node->next = NULL;
  return head;
}

int is_address_zero(struct ip_address_struct *ip_struct) {
  if (ip_struct->len == 4)  // IPv4
  {
    struct sockaddr_in *addr_in = (struct sockaddr_in *)&ip_struct->addr;
    struct in_addr zero_addr_in;
    memset(&zero_addr_in, 0, sizeof(zero_addr_in));

    return memcmp(&addr_in->sin_addr, &zero_addr_in, sizeof(struct in_addr)) ==
           0;
  } else if (ip_struct->len == 16)  // IPv6
  {
    struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)&ip_struct->addr;
    struct in6_addr zero_addr_in6;
    memset(&zero_addr_in6, 0, sizeof(zero_addr_in6));

    return memcmp(&addr_in6->sin6_addr, &zero_addr_in6,
                  sizeof(struct in6_addr)) == 0;
  } else {
    printf("Unknown address type\n");
    return -1;  // Unknown address type
  }
}

char *ip_to_string(void *ip_addr, int addr_type) {
  char *ip_string = NULL;

  if (addr_type == AF_INET) {  // IPv4
    ip_string = malloc(INET_ADDRSTRLEN);
    if (!ip_string) {
      perror("malloc");
      exit(EXIT_FAILURE);
    }
    if (inet_ntop(AF_INET, ip_addr, ip_string, INET_ADDRSTRLEN) == NULL) {
      perror("inet_ntop");
      exit(EXIT_FAILURE);
    }
  } else if (addr_type == AF_INET6) {  // IPv6
    ip_string = malloc(INET6_ADDRSTRLEN);
    if (!ip_string) {
      perror("malloc");
      exit(EXIT_FAILURE);
    }
    if (inet_ntop(AF_INET6, ip_addr, ip_string, INET6_ADDRSTRLEN) == NULL) {
      perror("inet_ntop");
      exit(EXIT_FAILURE);
    }
  }

  return ip_string;
}

void printIpList(ip_list list) {
  ip_list_node current = list;
  while (current != NULL) {
    char ipAddress[INET_ADDRSTRLEN];
    inet_ntop(AF_INET,
              &((struct sockaddr_in *)&current->ip_addr->addr)->sin_addr,
              ipAddress, INET_ADDRSTRLEN);
    printf("%s\n", ipAddress);
    current = current->next;
  }
}
