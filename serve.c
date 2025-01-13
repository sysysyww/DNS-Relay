#include <arpa/inet.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>

#include "cache.h"
#include "communication.h"
#include "dns_struct.h"
#include "encapsulation.h"
#include "hash_table.h"
#include "ip_address.h"
#include "log.h"
#include "parser.h"
extern hash_table_item hashTable[TABLE_SIZE];
extern pthread_mutex_t port_53_socket_lock;
extern pthread_mutex_t table_lock[TABLE_SIZE];
void service(void *arg)
{
    argument argu = (argument)arg;
    argu->id = (int)pthread_self();
    int tid = argu->id;
    LOG_VERBOSE("Thread %d: Strart a new task", tid);
    char *original_buf_ptr = argu->DNS_request;
    // 解析
    LOG_VERBOSE("Thread %d: Resoving packet...", tid);
    char domain[64];
    dns_header_t header;
    char *deep_copy = malloc(sizeof(char) * 1024);
    memcpy(deep_copy, argu->DNS_request, argu->request_length);
    int version = recv_parser(deep_copy, domain, &header);
    if (version == 1)
    {
        version = 4;
    }
    else if (version == 2)
    {
        version = 6;
    }
    else
    {
        LOG_VERBOSE("Thread %d: Resoving packet success, undefind type, returning...", tid);
        return;
    }

    LOG_VERBOSE("Thread %d: Resove success, domain name: %s, version: IPv%d", tid, domain, version);

    // 查缓存
    LOG_VERBOSE("Thread %d: Checking cache...", tid);
    time_t expiration_time;
    ip_list result_list = check_cache(domain, version, hashTable, &expiration_time);
    LOG_VERBOSE("Thread %d: Checking cache over", tid);
    if (result_list != NULL)
    {
        int invalid_flag = 0;
        ip_list_node node = result_list;
        while (node != NULL)
        {
            if (is_address_zero(node->ip_addr) == 1)
            {
                invalid_flag = 1;
                break;
            }
            node = node->next;
        }
        char *query = &(argu->DNS_request[12]);
        int response_size;
        if (invalid_flag)
        {
            // 1.1非法地址
            LOG_VERBOSE("Thread %d: Blocked domain name", tid);
            // 打包
            LOG_VERBOSE("Thread %d: Generating blocked domain name response...", tid);
            response *reply_error_response = generate_errorresponse(&header, query, result_list, expiration_time);
            char *error_reply = response_to_buffer(reply_error_response, &response_size);

            LOG_VERBOSE("Thread %d: Blocked domain name response generate success, %d byte", tid, response_size);
            // 返回（对socket上锁）
            pthread_mutex_lock(&port_53_socket_lock);
            LOG_VERBOSE("Thread %d: Sending blocked domain name response to client...", tid);
            send_back_to_client(error_reply, response_size, argu->client_addr, argu->client_socket, tid);
            LOG_VERBOSE("Thread %d: Blocked domain name response send success", tid);
            pthread_mutex_unlock(&port_53_socket_lock);
        }
        else
        {
            // 1.2合法地址
            LOG_VERBOSE("Thread %d: Cache hit", tid);
            // 打包
            LOG_VERBOSE("Thread %d: Generating DNS response...", tid);

            response *reply_response = generate_response(&header, query, result_list, expiration_time);
            char *reply = response_to_buffer(reply_response, &response_size);

            LOG_VERBOSE("Thread %d: Response generate success", tid);
            // 返回（对socket上锁）
            pthread_mutex_lock(&port_53_socket_lock);
            LOG_VERBOSE("Thread %d: Sending response to client...", tid);
            send_back_to_client(reply, response_size, argu->client_addr, argu->client_socket, tid);
            LOG_VERBOSE("Thread %d: Response send success", tid);
            pthread_mutex_unlock(&port_53_socket_lock);
        }
    }
    else
    {
        // 2.缓存不命中
        LOG_VERBOSE("Thread %d: Cache miss", tid);
        // forward-收到上游dns响应-返回给客户端
        char *response_for_forward_to_client = NULL;
        int response_length = forward(argu->DNS_request, argu->request_length, tid, &response_for_forward_to_client);
        pthread_mutex_lock(&port_53_socket_lock);
        LOG_VERBOSE("Thread %d: Sending upper DNS response to client...", tid);
        send_back_to_client(response_for_forward_to_client, response_length, argu->client_addr, argu->client_socket,
                            tid);
        LOG_VERBOSE("Thread %d: Upper DNS response send to client success", tid);
        pthread_mutex_unlock(&port_53_socket_lock);

        // 解析报文
        LOG_VERBOSE("Thread %d: Resolving upper DNS response...", tid);
        ip_list list = NULL;
        expiration_time = resolve_upper_response(response_for_forward_to_client, argu->request_length, &list);

        //  写缓存
        if (list != NULL)
        {
            LOG_VERBOSE("Thread %d: Resolve upper DNS response success", tid);
            hash_table_item item = create_new_hash_node(domain, list, expiration_time, version);
            unsigned int index = BKDRHash(domain, version);
            pthread_mutex_lock(&table_lock[index]);
            LOG_VERBOSE("Thread %d: Writing cache...", tid);
            int write_result = insert(index, item, hashTable);
            if (write_result == 0)
            {
                LOG_VERBOSE("Thread %d: Cache write success", tid);
            }
            else
                LOG_VERBOSE("Thread %d: Cache write fail, multiple write", tid);
            pthread_mutex_unlock(&(table_lock[index]));
        }
    }
}
