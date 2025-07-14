#ifndef SERVER_H
#define SERVER_H


#include "../include/global.h"
#include "../include/logger.h"


#define BACKLOG 10

char command[1024];
char server_ip[INET_ADDRSTRLEN];


int server_socket;
int server_listening_port;

ClientInfo clients[MAX_CLIENTS];
ActiveClientInfo active_clients[MAX_CLIENTS];
int max_fd;


void get_server_ip();
void handle_port();
void handle_ip();
void handle_author();
void handle_list();
void build_active_clients_list();
void handle_statistics();
bool does_ip_exist(char ip[IP_LENGTH]);
int handle_send_command(char client_ip[INET_ADDRSTRLEN], char *buffer);
int handle_block_command(char client_hostname[256], char *buffer);
int handle_unblock_command(char client_hostname[256], char *buffer);
void handle_get_blocked(char command[1024]);
ActiveClientInfo print_give_client_by_ip(char ip[IP_LENGTH]);
void handle_broadcast_command(char *sender_ip, char *buffer);
void start_server(int port);
void replay_msg(char client_hostname[256]);

#endif // SERVER_H
