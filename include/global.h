#ifndef GLOBAL_H_
#define GLOBAL_H_

#define HOSTNAME_LEN 128
#define PATH_LEN 256
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10
#define IP_LENGTH 16

#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/select.h>

typedef struct {
    char sender_ip[INET_ADDRSTRLEN];
    char msg[256];
} MsgObject;

typedef struct {
    int socket;
    struct sockaddr_in address;
    char hostname[BUFFER_SIZE];
    char ipaddress[BUFFER_SIZE];
    int port;
    int msg_sent;
    int msg_rsv;
    bool is_logged_in;
    char blocked_ips[MAX_CLIENTS][IP_LENGTH];
    MsgObject msgs[100];
} ClientInfo;

typedef struct {
    char hostname[BUFFER_SIZE];
    char ipaddress[BUFFER_SIZE];
    int port;
    bool is_logged_in;
} ActiveClientInfo;

fd_set master_fds, read_fds;  // File descriptor sets

#endif
