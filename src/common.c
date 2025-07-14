#include "../include/common.h"
#include "../include/logger.h"
#include <unistd.h>


void handle_author_command() {
    cse4589_print_and_log("[AUTHOR:SUCCESS]\n");
    cse4589_print_and_log("I, YOUR_TEAM_NAME, have read and understood the course academic integrity policy.\n");
    cse4589_print_and_log("[AUTHOR:END]\n");
}

void handle_ip_command() {
    char ip[INET_ADDRSTRLEN];
    struct sockaddr_in serv;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    serv.sin_family = AF_INET;
    serv.sin_port = htons(53);
    inet_pton(AF_INET, "8.8.8.8", &serv.sin_addr);
    connect(sock, (struct sockaddr*)&serv, sizeof(serv));
    socklen_t len = sizeof(serv);
    getsockname(sock, (struct sockaddr*)&serv, &len);
    inet_ntop(AF_INET, &serv.sin_addr, ip, sizeof(ip));
    close(sock);

    cse4589_print_and_log("[IP:SUCCESS]\n");
    cse4589_print_and_log("IP:%s\n", ip);
    cse4589_print_and_log("[IP:END]\n");
}

void handle_port_command(int port) {
    cse4589_print_and_log("[PORT:SUCCESS]\n");
    cse4589_print_and_log("PORT:%d\n", port);
    cse4589_print_and_log("[PORT:END]\n");
}

void handle_list_command(ClientInfo *clients, int count, int is_server) {
    cse4589_print_and_log("[LIST:SUCCESS]\n");
    for (int i = 0; i < count; i++) {
        if (clients[i].is_logged_in) {
            cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", i + 1, clients[i].hostname, clients[i].ip, clients[i].port);
        }
    }
    cse4589_print_and_log("[LIST:END]\n");
}
