#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>

#include "hash_table.h"
#include "ip_address.h"
#include "log.h"

extern char upper_server_address[16];

int forward(char *query, int query_len, int tid, char **response_to_client)
{
    LOG_VERBOSE("Thread %d: Creating socket to communicate with upper DNS server...", tid);

    struct sockaddr_in upstream_server_addr;
    memset(&upstream_server_addr, 0, sizeof(upstream_server_addr));
    upstream_server_addr.sin_family = AF_INET;
    upstream_server_addr.sin_port = htons(53);
    // Convert the IP address from a string
    if (inet_pton(AF_INET, upper_server_address, &(upstream_server_addr.sin_addr)) <= 0)
    {
        perror("inet_pton");
        return -1;
    }
    int upstream_socket = socket(PF_INET, SOCK_DGRAM, 0);
    if (upstream_socket < 0)
    {
        perror("socket() failed");
        return -1;
    }
    LOG_VERBOSE("Thread %d: Socket with upper DNS server create success", tid);
    LOG_VERBOSE("Thread %d: Forwarding request to upper DNS server...Destination %s:53", tid, upper_server_address);
    int num_bytes_sent = sendto(upstream_socket, query, query_len, 0, (struct sockaddr *)&upstream_server_addr,
                                sizeof(upstream_server_addr));

    if (num_bytes_sent != query_len)
    {
        perror("sendto() sent a different number of bytes than expected");
        return -1;
    }
    LOG_VERBOSE("Thread %d: Forward %d byte data success, ", tid, num_bytes_sent);
    LOG_SIMPLE("Forward DNS request to upper server")
    // Receive the response from the upstream server
    LOG_VERBOSE("Thread %d: Receiving response from upper DNS server...", tid);
    char *response = malloc(512); // 512 is the maximum size of a DNS message
    struct sockaddr_in response_addr;
    socklen_t response_addr_len = sizeof(response_addr);

    int num_bytes_received =
        recvfrom(upstream_socket, response, 512, 0, (struct sockaddr *)&response_addr, &response_addr_len);

    if (num_bytes_received < 0)
    {
        perror("recvfrom() failed");
        return -1;
    }
    LOG_VERBOSE("Thread %d: Receive response %d bytes from upper DNS server success", tid, num_bytes_received);
    LOG_SIMPLE("Receive DNS response from upper server")
    *response_to_client = response;
    close(upstream_socket);

    return num_bytes_received;
}

int send_back_to_client(char *response, int response_len, struct sockaddr_in *client_addr, int dns_socket, int tid)
{
    // send to client
    address client = convertIPAddress(*client_addr);

    LOG_VERBOSE("Thread %d: Sending response to client %s:%d...", tid, client->ip, client->port);
    LOG_SIMPLE("Send DNS response to client %s:%d", client->ip, client->port)
    if (response != NULL && client_addr != NULL)
    {
        int num_bytes_sent_to_client =
            sendto(dns_socket, response, response_len, 0, (struct sockaddr *)client_addr, sizeof(*client_addr));
        if (num_bytes_sent_to_client != response_len)
        {
            perror("sendto() sent a different number of bytes than expected");
            return -1;
        }
        LOG_VERBOSE("Thread %d: Send response %d byte to client success", tid, num_bytes_sent_to_client);
        return 0;
    }
    return -1;
}