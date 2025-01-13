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
                            time_t time1) {
  // 初始化response
  response *res = malloc(sizeof(response));
  // 设置header部分
  res->header = malloc(sizeof(dns_header_t));
  //   memcpy(res->header, header, sizeof(dns_header_t));
  res->header->id = ntohs(header->id);
  uint16_t flag = ntohs(header->flag);
  flag |= (1 << 15);                // 设置QR位为1
  flag &= 0xFFF0;                   // 设置Rcode为0
  flag &= ~(1 << 10);               // AA位置0
  flag |= (1 << 7);                 // RA位置1
  res->header->flag = htons(flag);  // 保留原来的flag的其他位
  res->header->question_count = htons(1);
  res->question = malloc(sizeof(dns_question));
  if (res->question == NULL) {
    fprintf(stderr, "Failed to allocate memory for DNS question.\n");
    exit(1);
  }
  // 设置查询部分
  char *name_end = strchr(query, '\0');
  res->question->name = malloc(name_end - query + 1);
  if (res->question->name == NULL) {
    fprintf(stderr, "Failed to allocate memory for DNS question name.\n");
    exit(1);
  }
  strncpy(res->question->name, query, name_end - query);
  res->question->name[name_end - query] = '\0';
  // 将query的类型部分存储在q_type字段中
  unsigned short q_type = *(unsigned short *)(name_end + 1);
  res->question->q_type = ntohs(q_type);
  // 将query的类别部分存储在q_class字段中
  unsigned short q_class = *(unsigned short *)(name_end + 3);
  res->question->q_class = ntohs(q_class);

  // 设置回答部分
  dns_rr *lastRR = NULL;
  while (ipList != NULL) {
    dns_rr *rr = malloc(sizeof(dns_rr));
    if (rr == NULL) {
      fprintf(stderr, "Failed to allocate memory for DNS RR.\n");
      exit(1);
    }

    rr->rName = res->question->name;
    rr->rType = res->question->q_type;
    rr->rClass = res->question->q_class;
    // struct tm *human_readable_time = localtime(&time1);

    // char buffer[80];
    // strftime(buffer, 80, "%Y-%m-%d %H:%M:%S",
    //          human_readable_time);  // 输出格式：年-月-日 时:分:秒
    // printf("in Formatted time: %s\n", buffer);
    rr->ttl = time1 - time(NULL);  // 这里假设你的time是一个未来的时间点

    struct sockaddr *sock_addr = (struct sockaddr *)&(ipList->ip_addr->addr);
    if (sock_addr->sa_family == AF_INET) {  // IPv4地址
      struct sockaddr_in *addr_in = (struct sockaddr_in *)sock_addr;
      rr->rdLen = 0x0004;  // IPv4地址长度为4字节
      rr->rData = malloc(4);
      if (rr->rData == NULL) {  // 检查内存分配是否成功
        fprintf(stderr, "Memory allocation failed.\n");
        free(rr);
        continue;
      }
      memcpy(rr->rData, &(addr_in->sin_addr), 4);
    } else if (sock_addr->sa_family == AF_INET6) {  // IPv6地址
      struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)sock_addr;
      rr->rdLen = 0x0010;  // IPv6地址长度为16字节
      rr->rData = malloc(16);
      if (rr->rData == NULL) {  // 检查内存分配是否成功
        fprintf(stderr, "Memory allocation failed.\n");
        free(rr);
        continue;
      }
      memcpy(rr->rData, &(addr_in6->sin6_addr), 16);
    } else {
      fprintf(stderr, "Unknown address family.\n");
      free(rr);
      continue;
    }

    rr->next = NULL;

    if (lastRR) {
      lastRR->next = rr;
    } else {
      res->firstRR = rr;
    }

    res->header->answer_count = htons(ntohs(res->header->answer_count) + 1);
    lastRR = rr;
    ipList = ipList->next;
  }

  // res->header->ns_count=0;
  // res->header->ar_count=0;
  return res;
}

response *generate_errorresponse(dns_header_t *header, char *query,
                                 ip_list ipList, time_t time1) {
  // 初始化response
  response *res = malloc(sizeof(response));
  // 设置header部分
  res->header = malloc(sizeof(dns_header_t));
  // memcpy(res->header, header, sizeof(dns_header_t));
  res->header->id = ntohs(header->id);
  uint16_t flag = ntohs(header->flag);
  flag |= (1 << 15);                // 设置QR位为1
  flag &= ~(0xF);                   // 清除原来的Rcode
  flag |= 3;                        // 设置新的Rcode为3
  flag &= ~(1 << 10);               // AA位置0
  flag |= (1 << 7);                 // RA位置1
  res->header->flag = htons(flag);  // 保留原来的flag的其他位
  res->header->question_count = htons(1);
  res->question = malloc(sizeof(dns_question));
  if (res->question == NULL) {
    fprintf(stderr, "Failed to allocate memory for DNS question.\n");
    exit(1);
  }
  // 设置查询部分
  char *name_end = strchr(query, '\0');
  res->question->name = malloc(name_end - query + 1);
  if (res->question->name == NULL) {
    fprintf(stderr, "Failed to allocate memory for DNS question name.\n");
    exit(1);
  }
  strncpy(res->question->name, query, name_end - query);
  res->question->name[name_end - query] = '\0';
  // 将query的类型部分存储在q_type字段中
  unsigned short q_type = *(unsigned short *)(name_end + 1);
  res->question->q_type = ntohs(q_type);
  // 将query的类别部分存储在q_class字段中
  unsigned short q_class = *(unsigned short *)(name_end + 3);
  res->question->q_class = ntohs(q_class);

  res->header->answer_count = htons(0);
  // res->header->ns_count=0;
  // res->header->ar_count=0;
  return res;
}

unsigned char *response_to_buffer(response *res, int *length) {
  // 首先计算需要的buffer大小
  int size = sizeof(dns_header_t) + strlen(res->question->name) + 1 +
             2 * sizeof(uint16_t);
  dns_rr *rr = res->firstRR;
  while (rr) {
    size += 12;
    if (rr->rdLen == 0x0004)  // IPv4地址长度为4字节
      size += 4;
    else if (rr->rdLen == 0x0010)  // IPv6地址长度为16字节
      size += 16;
    rr = rr->next;
  }
  *length = size;
  // 分配buffer
  size += 100000;
  unsigned char *buffer = malloc(size);
  if (buffer == NULL) {
    fprintf(stderr, "Failed to allocate memory for DNS response buffer.\n");
    exit(1);
  }
  // 将header写入buffer
  unsigned char *ptr = buffer;
  memcpy(ptr, res->header, sizeof(dns_header_t));
  ptr += sizeof(dns_header_t);
  // 将查询写入buffer
  strcpy((char *)ptr, res->question->name);
  ptr += strlen(res->question->name) + 1;
  *((uint16_t *)ptr) = htons(res->question->q_type);
  ptr += sizeof(uint16_t);
  *((uint16_t *)ptr) = htons(res->question->q_class);
  ptr += sizeof(uint16_t);
  // 将回答写入buffer
  rr = res->firstRR;
  while (rr) {
    size_t name_len = strlen(rr->rName);
    *((uint16_t *)ptr) = htons(0xC00C);  // 将0xC00C转换为网络字节序并写入buffer
    ptr += sizeof(uint16_t);  // 更新指针位置
    // 将其他字段复制到buffer，每次都更新指针
    *((uint16_t *)ptr) = htons(rr->rType);
    ptr += sizeof(uint16_t);
    *((uint16_t *)ptr) = htons(rr->rClass);
    ptr += sizeof(uint16_t);
    *((uint32_t *)ptr) = htonl(rr->ttl);
    ptr += sizeof(uint32_t);
    int rdLen;
    if (rr->rdLen == 0x0004) {
      *((uint16_t *)ptr) = htons(0x4);
      ptr += sizeof(uint16_t);
      rdLen = 4;  // 获取资源数据长度（字节数）
    } else if (rr->rdLen == 0x0010) {
      *((uint16_t *)ptr) = htons(0x10);
      ptr += sizeof(uint16_t);
      rdLen = 16;  // 获取资源数据长度（字节数）
    }

    // 确保资源数据不会溢出buffer
    if (rdLen > (buffer + size - ptr)) {  //
      fprintf(stderr, "Resource data is too large for buffer.\n");
      exit(1);
    }

    memcpy(ptr, rr->rData, rdLen);  // 复制资源数据到 buffer

    ptr += rdLen;  // 将 buffer 指针移动到下一个位置

    rr = rr->next;
  }
  return buffer;
}