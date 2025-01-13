#include <ctype.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "dns_struct.h"
#include "hash_table.h"
#include "log.h"
#include "pthread_pool.h"
#include "serve.h"

#define THREAD_NUM 1
#define SERVER_ADDRESS "172.20.10.9"
#define CACHE_FILE "dnsrelay.txt"

char upper_server_address[16] = "223.5.5.5";

pthread_mutex_t port_53_socket_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t table_lock[TABLE_SIZE];

hash_table_item hashTable[TABLE_SIZE] = {NULL};

int verbose_mode = 0;  // 默认不是详尽模式

int socket_bind() {
  LOG_VERBOSE("Creating socket for receiving...\n");
  LOG_SIMPLE("Creating socket for receiving...\n");

  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

  LOG_VERBOSE("Binding socket to server port 53...\n");
  LOG_SIMPLE("Binding socket to server port 53...\n");

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(53);
  if (inet_pton(AF_INET, SERVER_ADDRESS, &(addr.sin_addr)) != 1) {
    perror("inet_pton");
    return -1;
  }
  int ret = bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
  if (ret < 0) {
    printf("bind error\n");
    return -1;
  }

  LOG_VERBOSE("Bind successfully\n");
  LOG_SIMPLE("Bind successfully \n");
  return sockfd;
}

int main(int argc, const char *argv[]) {
  // set configuration
  if (strcmp(argv[2], "-d") == 0) {
    verbose_mode = 0;
    strcpy(upper_server_address, argv[3]);
  } else if (strcmp(argv[2], "-dd") == 0) {
    verbose_mode = 1;
    strcpy(upper_server_address, argv[3]);
  } else {
    printf("wrong argument!!!!\n");
    return -1;
  }

  // Initial threadpool
  LOG_VERBOSE("Initiating threadpool\n");
  LOG_SIMPLE("Initiating threadpool\n");
  threadpool *pool = thread_pool_initial(THREAD_NUM);

  // init cache
  load_hash(CACHE_FILE, hashTable);

  for (int i = 0; i < TABLE_SIZE; i++) {
    if (pthread_mutex_init(&table_lock[i], NULL) != 0) {
      printf("lock init error\n");
    }
  }

  int sockfd = socket_bind();

  while (1) {
    char buf[1024];
    struct sockaddr_in cli;
    socklen_t len = sizeof(cli);
    char *deep_copy_request = malloc(sizeof(char) * 1024);
    int byte_received =
        recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&cli, &len);
    memcpy(deep_copy_request, buf, byte_received);
    struct argument_struct *argv =
        (struct argument_struct *)malloc(sizeof(struct argument_struct));

    argv->DNS_request = deep_copy_request;
    argv->request_length = byte_received;
    argv->client_addr = &cli;
    argv->client_socket = sockfd;
    thread_pool_add(pool, service, (void *)argv);
  }
  thread_pool_destroy(pool);
  free(hashTable);
  for (int i = 0; i < TABLE_SIZE; i++) {
    pthread_mutex_destroy(&table_lock[i]);
  }
  pthread_mutex_destroy(&port_53_socket_lock);

  return 0;
}
