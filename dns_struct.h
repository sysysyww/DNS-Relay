#ifndef DNS_STRUCT_H
#define DNS_STRUCT_H

#include <stdint.h>

typedef struct argument_struct *argument;

struct argument_struct {
  char *DNS_request;
  int request_length;
  struct sockaddr_in *client_addr;
  int client_socket;
  int id;
};

typedef struct dns_header {
  uint16_t id;
  uint16_t flag;
  uint16_t question_count;
  uint16_t answer_count;
  uint16_t ns_count;
  uint16_t ar_count;
} dns_header_t;

typedef struct dns_rr {
  char *rName;
  short rType;
  short rClass;
  int ttl;
  short rdLen;
  char *rData;
  struct dns_rr *next;
} dns_rr;

typedef struct dns_question {
  short q_type;
  short q_class;
  char *name;  // 域名
} dns_question;

typedef struct response {
  dns_header_t *header;
  dns_question *question;
  dns_rr *firstRR;
} response;

#endif /* DNS_STRUCT_H */
