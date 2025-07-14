#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

typedef struct {
    int socket;
    char ip[INET_ADDRSTRLEN];
    char hostname[1024];
    int port;
    int num_msg_sent;
    int num_msg_rcv;
    int is_logged_in;
    int blocked_count;
    char blocked_clients[10][INET_ADDRSTRLEN];
} ClientInfo;

void handle_author_command();
void handle_ip_command();
void handle_port_command(int port);
void handle_list_command(ClientInfo *clients, int count, int is_server);
void handle_login_command(char *server_ip, int server_port);
void handle_logout_command();
void handle_statistics_command(ClientInfo *clients, int count);
void handle_blocked_command(ClientInfo *clients, int count, char *ip);

#endif
