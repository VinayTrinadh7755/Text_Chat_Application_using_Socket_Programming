#include "../include/client.h"

int server_socket = -1;
int received_client_count = 0;

int max_fd;
fd_set master_fds;
fd_set read_fds;

void receive_client_list(int server_socket)
{
    memset(received_clients, 0, sizeof(received_clients)); // Clear previous data

    // Receive the client list
    int bytes_received = recv(server_socket, received_clients, sizeof(received_clients), 0);
    if (bytes_received <= 0)
    {
        // printf("Error receiving client list.\n");
        return;
    }

    // Calculate the number of received clients
    received_client_count = bytes_received / sizeof(ActiveClientInfo);

    // cse4589_print_and_log(" Reci client count %d, bytes rec %d, Size of clientinfo %d\n",
    //    received_client_count, bytes_received, sizeof(ActiveClientInfo));

    // printf("LIST start\n");
    // for (int i = 0; i < received_client_count; i++) {
    //     cse4589_print_and_log("%-35s%-20s%-8d\n",
    //            received_clients[i].hostname, received_clients[i].ipaddress, received_clients[i].port);
    // }
    // printf("List end\n");
}

void handle_login(char *server_ip, int server_port)
{

    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        cse4589_print_and_log("[LOGIN:ERROR]\n[LOGIN:END]\n");
        return;
    }

    // Enable socket reuse
    int opt = 1;
    if (setsockopt(client_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        cse4589_print_and_log("[LOGIN:ERROR]\n[LOGIN:END]\n");
        close(client_socket);
        client_socket = -1;
        return;
    }

    // Bind the socket
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = INADDR_ANY;
    client_addr.sin_port = htons(client_listening_port);

    if (bind(client_socket, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
    {
        // cse4589_print_and_log("Error: Bind failed");
        close(client_socket);
        exit(1);
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0)
    {
        cse4589_print_and_log("[LOGIN:ERROR]\n[LOGIN:END]\n");
        close(client_socket);
        return;
    }

    // Attempt to connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        cse4589_print_and_log("[LOGIN:ERROR]\n[LOGIN:END]\n");
        close(client_socket);
        return;
    }

    FD_SET(client_socket, &master_fds);
    max_fd = client_socket > max_fd ? client_socket : max_fd;

    receive_client_list(client_socket);

    server_socket = client_socket;
}

void handle_logout()
{
    if (client_socket == -1)
    {
        cse4589_print_and_log("[LOGOUT:ERROR]\n[LOGOUT:END]\n");
        return;
    }
    // backup_clientsocket = client_socket;
    // Send logout message to the server
    char logout_msg[] = "LOGOUT";
    send(client_socket, logout_msg, strlen(logout_msg), 0);

    // Close the client-server connection
    close(client_socket);
    FD_CLR(client_socket, &master_fds); // Clear the socket from master_fds
    client_socket = -1;                 // Mark the client as logged out
    max_fd = STDIN_FILENO;              // Reset max_fd to standard input

    cse4589_print_and_log("[LOGOUT:SUCCESS]\n");
    cse4589_print_and_log("[LOGOUT:END]\n");
}

void get_client_ip()
{
    char hostname[1024];
    struct addrinfo hints, *res;

    // Step 1: Get the hostname of the machine
    if (gethostname(hostname, sizeof(hostname)) == -1)
    {
        perror("Error: gethostname failed");
        strcpy(client_ip, "Unknown");
        return;
    }

    // Step 2: Set up hints for address resolution
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM;

    // Step 3: Get the IP address using getaddrinfo
    if (getaddrinfo(hostname, NULL, &hints, &res) != 0)
    {
        perror("Error: getaddrinfo failed");
        strcpy(client_ip, "Unknown");
        return;
    }

    // Step 4: Convert binary IP to human-readable format
    struct sockaddr_in *addr = (struct sockaddr_in *)res->ai_addr;
    inet_ntop(AF_INET, &addr->sin_addr, client_ip, INET_ADDRSTRLEN);

    freeaddrinfo(res); // Free memory
}

void handle_port_client()
{
    cse4589_print_and_log("[PORT:SUCCESS]\n");
    cse4589_print_and_log("PORT:%d\n", client_listening_port);
    cse4589_print_and_log("[PORT:END]\n");
}

void handle_ip_client()
{
    cse4589_print_and_log("[IP:SUCCESS]\n");
    get_client_ip();
    cse4589_print_and_log("IP:%s\n", client_ip);
    cse4589_print_and_log("[IP:END]\n");
}

void handle_author_client()
{
    cse4589_print_and_log("[AUTHOR:SUCCESS]\n");
    cse4589_print_and_log("I, hayenugu-vinaytri, have read and understood the course academic integrity policy.\n");
    cse4589_print_and_log("[AUTHOR:END]\n");
}

// CODE CHANGES
void handle_list_client()
{

    cse4589_print_and_log("[LIST:SUCCESS]\n");
    // Sort the received clients by their listening port number in increasing order
    for (int i = 0; i < received_client_count - 1; i++)
    {
        for (int j = 0; j < received_client_count - i - 1; j++)
        {
            if (received_clients[j].port > received_clients[j + 1].port)
            {
                // Swap the clients
                ActiveClientInfo temp = received_clients[j];
                received_clients[j] = received_clients[j + 1];
                received_clients[j + 1] = temp;
            }
        }
    }

    // Print the sorted list of clients with list ID
    int index = 0;
    for (int i = 0; i < received_client_count; i++)
    {
        if (received_clients[i].is_logged_in)
        {
            cse4589_print_and_log("%-5d%-35s%-20s%-8d\n",
                                  index + 1, received_clients[i].hostname, received_clients[i].ipaddress, received_clients[i].port);
            index++;
        }
    }
    cse4589_print_and_log("[LIST:END]\n");
}

void start_client(int port)
{

    client_listening_port = port;
    client_socket = -1;
    // struct sockaddr_in server_addr, client_addr;
    // socklen_t client_len = sizeof(client_addr);

    // Initialize max_fd and file descriptor sets

    FD_ZERO(&master_fds);
    FD_SET(STDIN_FILENO, &master_fds); // Monitor standard input
    max_fd = STDIN_FILENO;

    while (1)
    {

        read_fds = master_fds;
        // Monitor standard input and server socket
        if (client_socket != -1)
        {
            FD_SET(client_socket, &master_fds);
            max_fd = client_socket > STDIN_FILENO ? client_socket : STDIN_FILENO;
        }
        else
        {
            max_fd = STDIN_FILENO;
        }
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0)
        {
            // perror("Error: select() failed");
            continue;
        }

        // Check if there is input from the user
        if (FD_ISSET(STDIN_FILENO, &read_fds))
        {
            // cse4589_print_and_log("Inside while, max_fd : %d\n", max_fd);
            char command[BUFFER_SIZE];
            fgets(command, BUFFER_SIZE, stdin);
            command[strcspn(command, "\n")] = 0;

            if (strcmp(command, "EXIT") == 0)
            {
                send(client_socket, command, strlen(command), 0);
                break;
            }
            else if (strcmp(command, "AUTHOR") == 0)
            {
                handle_author_client();
            }
            else if (strcmp(command, "IP") == 0)
            {
                handle_ip_client();
            }
            else if (strcmp(command, "PORT") == 0)
            {
                handle_port_client();
            }
            else if (strcmp(command, "LIST") == 0)
            {
                if (client_socket != -1)
                    handle_list_client();
                else
                {
                    cse4589_print_and_log("[LIST:SUCCESS]\n[LIST:END]\n");
                }
            }
            else if (strcmp(command, "LOGOUT") == 0)
            {
                handle_logout();
            }
            else if (strncmp(command, "BLOCK", 5) == 0)
            {
                char ip_to_block[IP_LENGTH];
                memset(ip_to_block, 0, sizeof(ip_to_block));
                struct sockaddr_in sa;
                sscanf(command + 5, "%s", ip_to_block);
                int result = inet_pton(AF_INET, ip_to_block, &(sa.sin_addr));
                // cse4589_print_and_log("IP to Block %s and status: %d\n", ip_to_block, result);
                if (result == 1 && client_socket != -1)
                { // Ensure client is connected
                    send(client_socket, command, strlen(command), 0);
                    // cse4589_print_and_log("[BLOCK:SUCCESS]\n[BLOCK:END]\n");
                }
                else
                {
                    cse4589_print_and_log("[BLOCK:ERROR]\n[BLOCK:END]\n");
                }
            }
            else if (strncmp(command, "UNBLOCK", 7) == 0)
            {
                char ip_to_block[IP_LENGTH];
                memset(ip_to_block, 0, sizeof(ip_to_block));
                struct sockaddr_in sa;
                sscanf(command + 7, "%s", ip_to_block);
                int result = inet_pton(AF_INET, ip_to_block, &(sa.sin_addr));
                if (result == 1 && client_socket != -1)
                { // Ensure client is connected
                    send(client_socket, command, strlen(command), 0);
                    // cse4589_print_and_log("[UNBLOCK:SUCSESS]\n[UNBLOCK:END]\n");
                }
                else
                {
                    cse4589_print_and_log("[UNBLOCK:ERROR]\n[UNBLOCK:END]\n");
                }
            }
            else if (strcmp(command, "REFRESH") == 0)
            {
                if (client_socket != -1)
                { // Ensure client is connected
                    send(client_socket, command, strlen(command), 0);
                    receive_client_list(client_socket);
                    cse4589_print_and_log("[REFRESH:SUCCESS]\n[REFRESH:END]\n");
                }
                else
                {
                    cse4589_print_and_log("[REFRESH:ERROR]\n[REFRESH:END]\n");
                }
            }
            else if (strncmp(command, "LOGIN", 5) == 0)
            {
                char server_ip[INET_ADDRSTRLEN];
                int server_port;
                if (sscanf(command, "LOGIN %s %d", server_ip, &server_port) == 2)
                {
                    handle_login(server_ip, server_port);
                    send(client_socket, "REPLAY", strlen("REPLAY"), 0);
                    // cse4589_print_and_log("[LOGIN:SUCCESS]\n");
                    // // cse4589_print_and_log("Connected to server at %s:%d\n", server_ip, server_port);
                    // cse4589_print_and_log("[LOGIN:END]\n");
                }
                else
                {
                    cse4589_print_and_log("[LOGIN:ERROR]\n[LOGIN:END]\n");
                }
            }
            else if (strncmp(command, "SEND", 4) == 0)
            {
                if (client_socket != -1)
                { // Ensure client is connected
                    send(client_socket, command, strlen(command), 0);
                    // cse4589_print_and_log("[SEND:SUCCESS]\n[SEND:END]\n");
                }
                else
                {
                    cse4589_print_and_log("[SEND:ERROR]\n[SEND:END]\n");
                }
            }
            else if (strncmp(command, "BROADCAST", 9) == 0)
            {
                if (client_socket != -1)
                { // Ensure client is connected
                    send(client_socket, command, strlen(command), 0);
                    cse4589_print_and_log("[BROADCAST:SUCCESS]\n[BROADCAST:END]\n");
                }
                else
                {
                    cse4589_print_and_log("[BROADCAST:ERROR]\n[BROADCAST:END]\n");
                }
            }
        }

        // Check if there is a message from the server
        // Check if there is a message from the server
        // cse4589_print_and_log(" client_socket : %d, FD_ISSET %d\n", client_socket, FD_ISSET(client_socket, &read_fds));
        if (client_socket != -1 && FD_ISSET(client_socket, &read_fds))
        {
            // cse4589_print_and_log("2nd if , max_fd : %d\n", max_fd);
            char buffer[BUFFER_SIZE];
            memset(buffer, 0, sizeof(buffer)); // Properly clear the buffer before receiving

            int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

            if (bytes_received <= 0)
            {
                // cse4589_print_and_log("Server disconnected.\n");
                close(client_socket);
                client_socket = -1;
            }
            else
            {
                buffer[bytes_received] = '\0'; // Null-terminate the received data
                // cse4589_print_and_log("Buffer : %s\n", buffer);
                // Split multiple messages if concatenated
                char *message = strtok(buffer, "\n");
                while (message != NULL)
                {
                    // cse4589_print_and_log("*** %s\n", message);
                    if (strncmp(message, "RECEIVED", 8) == 0)
                    {
                        cse4589_print_and_log("[RECEIVED:SUCCESS]\n");
                        // Extract sender IP and message
                        char sender_ip[INET_ADDRSTRLEN], msg[BUFFER_SIZE];
                        sscanf(message + 9, "%s %[^\n]", sender_ip, msg);
                        cse4589_print_and_log("msg from:%s\n[msg]:%s\n", sender_ip, msg);
                        cse4589_print_and_log("[RECEIVED:END]\n");
                    }
                    else if (strncmp(message, "BLOCK", 5) == 0)
                    {
                        char status[256];
                        sscanf(message + 5, "%s", status);
                        // cse4589_print_and_log("received: %s from %s\n", status, message);
                        if (strcmp(status, "SUCCESS") == 0)
                        {
                            cse4589_print_and_log("[BLOCK:SUCCESS]\n[BLOCK:END]\n");
                        }
                        else
                        {
                            cse4589_print_and_log("[BLOCK:ERROR]\n[BLOCK:END]\n");
                        }
                    }
                    else if (strncmp(message, "UNBLOCK", 7) == 0)
                    {
                        char status[256];
                        sscanf(message + 7, "%s", status);
                        // cse4589_print_and_log("received: %s from %s\n", status, message);
                        if (strcmp(status, "SUCCESS") == 0)
                        {
                            cse4589_print_and_log("[UNBLOCK:SUCCESS]\n[UNBLOCK:END]\n");
                        }
                        else
                        {
                            cse4589_print_and_log("[UNBLOCK:ERROR]\n[UNBLOCK:END]\n");
                        }
                    }
                    else if (strncmp(message, "SEND", 4) == 0)
                    {
                        char status[256];
                        sscanf(message + 4, "%s", status);
                        // cse4589_print_and_log("received: %s from %s\n", status, message);
                        if (strcmp(status, "SUCCESS") == 0)
                        {
                            cse4589_print_and_log("[SEND:SUCCESS]\n[SEND:END]\n");
                        }
                        else
                        {
                            cse4589_print_and_log("[SEND:ERROR]\n[SEND:END]\n");
                        }
                    }
                    else if(strcmp(message, "SUCCESS") == 0){
                        cse4589_print_and_log("[LOGIN:SUCCESS]\n[LOGIN:END]\n");
                    }
                    message = strtok(NULL, "\n"); // Process next message if concatenated
                }
            }
        }
    }
}