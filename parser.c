#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "dns_struct.h"
#include "hash_table.h"
#include "ip_address.h"
int recv_parser(char *buffer, char *domain, struct dns_header *hdr)
{ //

    char *query = (char *)&buffer[12];
    int i = 0, j = 0, k = 0, l = 0, dot_num = 0;
    while (*query)
    {
        if (query[i] != 0) // 如果这个字节最高位为 0
        {
            l = i;
            j = 0;
            while (j < query[l]) // 读取下一个子级域名的长度
            {
                i++;
                domain[k++] = query[i];
                j++;
            }
            domain[k++] = '.';
            i++;
            dot_num++;
        }
        else
        {
            // i += 2;
            break;
        }
    }
    domain[k - 1] = '\0'; // 将最后一个 . 替换成字符串结束符

    int type_bit = i;

    int type = query[type_bit + 2]; // q_types

    struct dns_header *hdrr = (dns_header_t *)buffer;

    hdr->id = ntohs(hdrr->id);
    hdr->flag = ntohs(hdrr->flag);
    hdr->question_count = ntohs(hdrr->question_count);
    hdr->answer_count = ntohs(hdrr->answer_count);
    hdr->ns_count = ntohs(hdrr->ns_count);
    hdr->ar_count = ntohs(hdrr->ar_count);

    free(hdrr);

    if (type == 0x1c)
    {
        return 2;
    }
    else if (type == 0x01)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

time_t resolve_upper_response(char *response, int header_and_query_length, ip_list *list)
{
    uint16_t RRnumber = ntohs(*(uint16_t *)(response + 6));
    response += header_and_query_length; // 现在response指针指向第一个rr的开头了
    uint32_t ttl;
    int find_valid_ip_flag = 0;

    for (uint16_t i = 0; i < RRnumber; i++)
    {
        if (((unsigned char)(response[0])) >> 6 == 3)
        {
            response += 2; // 现在response指针指向类型了

            uint16_t type = ntohs(*(uint16_t *)response);
            if (type != 1 && type != 28) // 不是A或AAAA，就不解析，跳到资源数据长度
            {
                response += 2 + 2 + 4; // 现在指向资源数据长度了
                uint16_t resource_length = ntohs(*(uint16_t *)(response));
                response += 2 + resource_length; // 现在指向当前rr的结尾，也就是下一个rr的开头
            }
            else // 是A或AAAA，要解析，并添加到列表里
            {
                if (find_valid_ip_flag == 0) // 第一回找到合法的，需要把ttl填上
                {
                    response += 2 + 2; // 现在指针指向生存时间了
                    ttl = ntohl(*(uint32_t *)response);
                    response += 4; // 现在指向资源数据长度了
                }
                else // 不是第一回，就不用看ttl了
                {
                    response += 2 + 2 + 4; // 现在指向资源数据长度了
                }

                // 解析ip
                uint16_t resource_length = ntohs(*(uint16_t *)(response));
                response += 2; // 现在指向地址了
                char *address;
                if (resource_length == 4)
                {
                    address = ip_to_string(response, AF_INET);
                }
                else
                {
                    address = ip_to_string(response, AF_INET6);
                }

                // 添加到列表，如果是第一个，那就直接创建赋给*list，不然就要填在后面
                if (*list == NULL)
                {
                    *list = create_ip_list_node(address);
                }
                else
                {
                    *list = add_to_ip_list(*list, address);
                }

                // 解析完了，跳到下一个头
                response += resource_length;
            }
        }
        else
        {
            return 0;
        }
    }
    time_t expirationTime = time(NULL) + (time_t)ttl;
    return expirationTime;
}