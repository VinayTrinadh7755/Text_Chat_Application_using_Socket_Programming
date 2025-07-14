#ifndef CLIENT_H
#define CLIENT_H

#include "../include/global.h"
#include "../include/logger.h"

char client_ip[INET_ADDRSTRLEN];
char command[1024];

int client_socket;
int client_listening_port;
// int backup_clientsocket;

ActiveClientInfo received_clients[MAX_CLIENTS];  // Array to store received clients


void receive_client_list(int server_socket);
void get_client_ip();
void handle_port_client();
void handle_ip_client();
void handle_author_client();
void start_client(int port);
void handle_login(char *server_ip, int server_port);
void handle_logout();
void handle_list_client();
void handle_broadcast(char *message);

#endif // CLIENT_H
