#include "../include/server.h"

// Declare `process_client_command` at the top
// void process_client_command(int client_socket, char *buffer);

/**
 * Start Server (Accept Connections & Handle Clients)
 */
int client_count = 0;
void replay_msg(char client_hostname[256])
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        // cse4589_print_and_log("Inside relay_msg for loop1\n");
        if (strcmp(clients[i].hostname, client_hostname) == 0 && clients[i].is_logged_in)
        {
            for (int k = 0; k < 100; k++)
            {
                if (strlen(clients[i].msgs[k].sender_ip) != 0 && strlen(clients[i].msgs[k].msg) != 0)
                {
                    // cse4589_print_and_log("%s %s\n", clients[i].msgs[k].sender_ip, clients[i].msgs[k].msg);
                    char msg_send[1024];
                    snprintf(msg_send, sizeof(msg_send), "RECEIVED %s %s\n", clients[i].msgs[k].sender_ip, clients[i].msgs[k].msg);
                    // cse4589_print_and_log("Message to send : %s\n", msg_send);
                    if (clients[i].socket == -1)
                    {
                        cse4589_print_and_log("Error: Invalid socket for client %s\n", client_hostname);
                        continue;
                    }
                    // if (send(clients[i].socket, "This is Testing", 14, MSG_NOSIGNAL) == -1)
                    // {
                    //     cse4589_print_and_log("Something went wrong with testing\n");
                    // }
                    if (send(clients[i].socket, msg_send, strlen(msg_send), MSG_NOSIGNAL) == -1)
                    {
                        // perror(" Error: Failed to send message");
                        // cse4589_print_and_log("Something went wrong\n");
                        return;
                    }
                    else
                    {
                        // cse4589_print_and_log("Clearing %s %s\n", clients[i].msgs[k].sender_ip, clients[i].msgs[k].msg);
                        memset(&clients[i].msgs[k], 0, sizeof(MsgObject));
                    }
                }
            }
        }
    }
}
void get_server_ip()
{
    char hostname[1024];
    struct addrinfo hints, *res;

    // Step 1: Get the hostname of the machine
    if (gethostname(hostname, sizeof(hostname)) == -1)
    {
        // perror("Error: gethostname failed");
        strcpy(server_ip, "Unknown");
        return;
    }

    // Step 2: Set up hints for address resolution
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM;

    // Step 3: Get the IP address using getaddrinfo
    if (getaddrinfo(hostname, NULL, &hints, &res) != 0)
    {
        // perror("Error: getaddrinfo failed");
        strcpy(server_ip, "Unknown");
        return;
    }

    // Step 4: Convert binary IP to human-readable format
    struct sockaddr_in *addr = (struct sockaddr_in *)res->ai_addr;
    inet_ntop(AF_INET, &addr->sin_addr, server_ip, INET_ADDRSTRLEN);

    freeaddrinfo(res); // Free memory
}

void handle_port()
{
    cse4589_print_and_log("[PORT:SUCCESS]\n");
    cse4589_print_and_log("PORT:%d\n", server_listening_port);
    cse4589_print_and_log("[PORT:END]\n");
}

void handle_ip()
{
    cse4589_print_and_log("[IP:SUCCESS]\n");
    get_server_ip();
    cse4589_print_and_log("IP:%s\n", server_ip);
    cse4589_print_and_log("[IP:END]\n");
}

void handle_author()
{
    cse4589_print_and_log("[AUTHOR:SUCCESS]\n");
    cse4589_print_and_log("I, hayenugu-vinaytri, have read and understood the course academic integrity policy.\n");
    cse4589_print_and_log("[AUTHOR:END]\n");
}

void build_active_clients_list()
{
    // clear the active clients
    memset(active_clients, 0, sizeof(active_clients));
    // build it from clients list
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        // cse4589_print_and_log("Hostname: %s, is logged in : %s\n", clients[i].hostname, clients[i].is_logged_in?"true":"False");
        if (clients[i].is_logged_in)
        {
            // cse4589_print_and_log("Inside Hostname %s\n", clients[i].hostname);
            // add to active client list
            // active_clients[i] = clients[i];
            strcpy(active_clients[i].hostname, clients[i].hostname);
            strcpy(active_clients[i].ipaddress, clients[i].ipaddress);
            active_clients[i].port = clients[i].port;
            active_clients[i].is_logged_in = true;
        }
    }
}

void handle_list()
{
    build_active_clients_list();

    cse4589_print_and_log("[LIST:SUCCESS]\n");

    // Sort active_clients by port number (Bubble Sort)
    for (int i = 0; i < client_count - 1; i++)
    {
        for (int j = 0; j < client_count - i - 1; j++)
        {
            if (active_clients[j].port > active_clients[j + 1].port)
            {
                // Swap clients[j] and clients[j + 1]
                ActiveClientInfo temp = active_clients[j];
                active_clients[j] = active_clients[j + 1];
                active_clients[j + 1] = temp;
            }
        }
    }

    // Print sorted list using the given format
    // printf("Client count %d\n", client_count);
    int index = 0;
    for (int i = 0; i < client_count; i++)
    {
        // printf("is logged in %s\n", active_clients[i].is_logged_in ? "true" : "false");
        if (active_clients[i].is_logged_in)
        {
            cse4589_print_and_log("%-5d%-35s%-20s%-8d\n",
                                  index + 1,                   // list_id (1-based index)
                                  active_clients[i].hostname,  // hostname
                                  active_clients[i].ipaddress, // IP address
                                  active_clients[i].port       // Port number
            );
            index++;
        }
    }
    cse4589_print_and_log("[LIST:END]\n");
}

int handle_send_command(char client_ip[INET_ADDRSTRLEN], char *buffer)
{
    char target_ip[INET_ADDRSTRLEN], message[256];
    // Extract target IP and message from buffer
    if (sscanf(buffer, "SEND %s %[^\n]", target_ip, message) != 2)
    {
        // cse4589_print_and_log("[SEND:ERROR]\n[SEND:END]\n");
        return 0;
    }
    // Validate IP format
    struct sockaddr_in temp_addr;
    if (inet_pton(AF_INET, target_ip, &temp_addr.sin_addr) <= 0)
    {
        // cse4589_print_and_log("[SEND:ERROR]\n[SEND:END]\n");
        return 0;
    }
    // Find the recipient
    for (int i = 0; i < client_count; i++)
    {
        // cse4589_print_and_log("comparing: %s and %s\n", clients[i].ipaddress, target_ip);
        bool flag = false;
        // check if target_ip address is a logged-in client
        if (strcmp(clients[i].ipaddress, target_ip) == 0 && clients[i].is_logged_in)
        {
            // check if sender is blocked
            for (int j = 0; j < MAX_CLIENTS; j++)
            {
                // cse4589_print_and_log("comparing: %s and %s\n", clients[i].blocked_ips[j], client_ip);
                if (strcmp(clients[i].blocked_ips[j], client_ip) == 0)
                    flag = true;
            }
            // Use MSG_NOSIGNAL to prevent crashes due to broken pipe errors
            if (!flag)
            {
                char msg_send[1024];
                snprintf(msg_send, sizeof(msg_send), "RECEIVED %s %s\n", client_ip, message);
                if (send(clients[i].socket, msg_send, strlen(msg_send), MSG_NOSIGNAL) == -1)
                {
                    // perror(" Error: Failed to send message");
                    // cse4589_print_and_log("[SEND:ERROR]\n[SEND:END]\n");
                    return 0;
                }

                cse4589_print_and_log("[RELAYED:SUCCESS]\n");
                cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", client_ip, target_ip, message);
                cse4589_print_and_log("[RELAYED:END]\n");
                clients[i].msg_rsv++;
                return 1;
            }
            return 1; // return 1 as the target_ip valid but sender is blocked

            // Print confirmation
            // cse4589_print_and_log("[SEND:SUCCESS]\n[SEND:END]\n");
            // return 0;
        }
        else if (strcmp(clients[i].ipaddress, target_ip) == 0 && !clients[i].is_logged_in)
        {
            // this block executes when target ip address is valid and logged out
            // check if sender is blocked 
            for (int j = 0; j < MAX_CLIENTS; j++)
            {
                // cse4589_print_and_log("comparing: %s and %s\n", clients[i].blocked_ips[j], client_ip);
                if (strcmp(clients[i].blocked_ips[j], client_ip) == 0)
                    flag = true;
            }
            if (!flag)
            {
                for (int k = 0; k < 100; k++)
                {
                    if (strlen(clients[i].msgs[k].sender_ip) == 0 && strlen(clients[i].msgs[k].msg) == 0)
                    {
                        strcpy(clients[i].msgs[k].msg, message);
                        strcpy(clients[i].msgs[k].sender_ip, client_ip);
                        return 1;
                    }
                }
            }
            return 1;
        }
    }
    return 0;

}

// CODE CHANGES
void handle_statistics()
{
    // Temporary array to hold the sorted list of clients
    ClientInfo sorted_clients[MAX_CLIENTS];
    memcpy(sorted_clients, clients, sizeof(clients));
    // Sort the clients by their port numbers
    for (int i = 0; i < client_count - 1; i++)
    {
        for (int j = i + 1; j < client_count; j++)
        {
            if (sorted_clients[i].port > sorted_clients[j].port)
            {
                // Swap the clients
                ClientInfo temp = sorted_clients[i];
                sorted_clients[i] = sorted_clients[j];
                sorted_clients[j] = temp;
            }
        }
    }
    // Print the sorted list
    cse4589_print_and_log("[STATISTICS:SUCCESS]\n");
    int index = 0;
    for (int i = 0; i < client_count; i++)
    {
        if (sorted_clients[i].hostname[0] != '\0')
        {
            cse4589_print_and_log("%-5d%-35s%-8d%-8d%-8s\n",
                                  (index + 1),
                                  sorted_clients[i].hostname,
                                  sorted_clients[i].msg_sent,
                                  sorted_clients[i].msg_rsv,
                                  sorted_clients[i].is_logged_in ? "logged-in" : "logged-out");
            index++;
        }
    }
    cse4589_print_and_log("[STATISTICS:END]\n");
}

bool does_ip_exist(char ip[IP_LENGTH])
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        // cse4589_print_and_log("client ipaddress %s, ip %s\n", clients[i].ipaddress, ip);
        if (strcmp(clients[i].ipaddress, ip) == 0)
        {
            return true;
        }
    }
    return false;
}
int handle_block_command(char client_hostname[256], char *buffer)
{
    char target_ip[INET_ADDRSTRLEN];
    // Extract target IP and message from buffer
    if (sscanf(buffer, "BLOCK %s", target_ip) != 1)
    {
        return 0;
    }
    // start
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, target_ip, &(sa.sin_addr));
    bool block_success = false;
    if (!does_ip_exist(target_ip))
        return 0;
    // end
    // cse4589_print_and_log("target_ip: %s, is_valid_ip: %d\n", target_ip, result);
    for (int i = 0; result == 1 && i < MAX_CLIENTS; i++)
    {
        if (strcmp(clients[i].hostname, client_hostname) == 0)
        {
            for (int j = 0; j < MAX_CLIENTS; j++)
            {
                if (strcmp(clients[i].blocked_ips[j], target_ip) == 0)
                {
                    return 0;
                }
            }
            for (int j = 0; j < MAX_CLIENTS; j++)
            {
                if (strcmp(clients[i].blocked_ips[j], "") == 0)
                {
                    strcpy(clients[i].blocked_ips[j], target_ip);
                    block_success = true;
                    // cse4589_print_and_log("Blocked: %s for %s\n", clients[i].blocked_ips[j], client_hostname);
                    break;
                }
            }
        }
    }

    if (!block_success)
    {
        return 0;
    }
    return 1;
}

int handle_unblock_command(char client_hostname[256], char *buffer)
{
    char target_ip[INET_ADDRSTRLEN];
    // Extract target IP and message from buffer
    if (sscanf(buffer, "UNBLOCK %s", target_ip) != 1)
    {
        return 0;
    }

    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, target_ip, &(sa.sin_addr));
    bool unblock_success = false;
    if (!does_ip_exist(target_ip))
        return 0;

    for (int i = 0; result == 1 && i < MAX_CLIENTS; i++)
    {
        if (strcmp(clients[i].hostname, client_hostname) == 0)
        {
            for (int j = 0; j < MAX_CLIENTS; j++)
            {
                if (strcmp(clients[i].blocked_ips[j], target_ip) == 0)
                {
                    strcpy(clients[i].blocked_ips[j], "");
                    unblock_success = true;
                    break;
                }
            }
        }
    }
    if (!unblock_success)
    {
        return 0;
    }
    return 1;
}

ActiveClientInfo print_give_client_by_ip(char ip[IP_LENGTH])
{
    // int index = 0;
    ActiveClientInfo temp;
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (strcmp(clients[i].ipaddress, ip) == 0)
        {
            // cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", index + 1, clients[i].hostname, clients[i].ipaddress, clients[i].port);
            strcpy(temp.hostname, clients[i].hostname);
            strcpy(temp.ipaddress, clients[i].ipaddress);
            temp.port = clients[i].port;
            temp.is_logged_in = clients[i].is_logged_in;
            // index++;
        }
    }
    // cse4589_print_and_log("[BLOCKED:END]\n");
    return temp;
}

void handle_get_blocked(char command[1024])
{
    char target_ip[INET_ADDRSTRLEN];
    // Extract target IP and message from buffer
    if (sscanf(command, "BLOCKED %s", target_ip) != 1)
    {
        cse4589_print_and_log("[BLOCKED:ERROR]\n[BLOCKED:END]\n");
        return;
    }
    ActiveClientInfo res[10];
    memset(res, 0, sizeof(res));
    int index = 0;
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (strcmp(clients[i].ipaddress, target_ip) == 0)
        {
            for (int j = 0; j < MAX_CLIENTS; j++)
            {
                // cse4589_print_and_log("clients blocked ip %s, bool %s\n", clients[i].blocked_ips[j], (strcmp(clients[i].blocked_ips[j], "") != 0) ? "true" : "False");
                if (strcmp(clients[i].blocked_ips[j], "") != 0)
                {
                    ActiveClientInfo temp = print_give_client_by_ip(clients[i].blocked_ips[j]);

                    strcpy(res[index].hostname, temp.hostname);
                    strcpy(res[index].ipaddress, temp.ipaddress);
                    res[index].port = temp.port;
                    res[index].is_logged_in = temp.is_logged_in;
                    index++;
                }
            }
        }
    }
    cse4589_print_and_log("[BLOCKED:SUCCESS]\n");

    // Sort active_clients by port number (Bubble Sort)

    /* index = 3
    4 1 2


    */

    //    for (int i = 0; i < MAX_CLIENTS; i++)
    //    {
    //         cse4589_print_and_log("res ip %s\n",res[i].ipaddress);
    //    }
    for (int i = 0; i < index; i++)
    {
        for (int j = 0; j < index - i - 1; j++)
        {
            if (res[j].port > res[j + 1].port)
            {
                // Swap clients[j] and clients[j + 1]
                ActiveClientInfo temp = res[j];
                res[j] = res[j + 1];
                res[j + 1] = temp;
            }
        }
    }

    // Print sorted list using the given format
    int idx = 0;
    for (int i = 0; i <= index; i++)
    {
        // printf("is logged in %s\n", res[i].is_logged_in ? "true" : "false");
        if (strcmp(res[i].hostname, "") != 0)
        {
            cse4589_print_and_log("%-5d%-35s%-20s%-8d\n",
                                  idx + 1,          // list_id (1-based index)
                                  res[i].hostname,  // hostname
                                  res[i].ipaddress, // IP address
                                  res[i].port       // Port number
            );
            idx++;
        }
    }
    cse4589_print_and_log("[BLOCKED:END]\n");
}

void handle_broadcast_command(char *sender_ip, char *buffer)
{
    char message[256];

    char msg_send[1024];
    memset(msg_send, 0, sizeof(msg_send));
    // Extract the broadcast message
    if (sscanf(buffer, "BROADCAST %[^\n]", message) != 1)
    {
        cse4589_print_and_log("[BROADCAST:ERROR]\n[BROADCAST:END]\n");
        return;
    }

    // char msg_send[1024];
    // strcat(msg_send, "RECEIVED ");
    // strcat(msg_send, client_ip);
    // strcat(msg_send, " ");
    // strcat(msg_send, message);

    snprintf(msg_send, sizeof(msg_send), "RECEIVED %s %s\n", sender_ip, message);

    // cse4589_print_and_log("Relaying msg: %s", msg_send);

    // Loop through all logged-in clients and send the message
    // cse4589_print_and_log("Sender ip: %s", sender_ip);
    bool send_atleast_once = false;
    for (int i = 0; i < client_count; i++)
    {
        // Skip the sender and logged-out clients
        bool flag = false;
        // cse4589_print_and_log("comparing: %d and %s Hostname : %s client ip : %s\n", strcmp(clients[i].ipaddress, sender_ip), clients[i].is_logged_in?"true":"false", clients[i].hostname, clients[i].ipaddress);
        if (clients[i].is_logged_in && strcmp(clients[i].ipaddress, sender_ip) != 0)
        {
            // cse4589_print_and_log("sending message to: %s\n", clients[i].ipaddress);
            for (int j = 0; j < MAX_CLIENTS; j++)
            {
                if (strcmp(clients[i].blocked_ips[j], sender_ip) == 0)
                    flag = true;
            }

            if (!flag)
            {
                if (send(clients[i].socket, msg_send, strlen(msg_send), MSG_NOSIGNAL) == -1)
                {
                    // perror("Error: Failed to send broadcast message");
                    continue;
                }
                clients[i].msg_rsv++;
                send_atleast_once = true;
            }
        }

        else if (strcmp(clients[i].ipaddress, sender_ip) != 0 && !clients[i].is_logged_in)
        {
            // cse4589_print_and_log("To store message\n");
            // check if sender is blocked
            for (int j = 0; j < MAX_CLIENTS; j++)
            {
                // cse4589_print_and_log("comparing: %s and %s\n", clients[i].blocked_ips[j], client_ip);
                if (strcmp(clients[i].blocked_ips[j], sender_ip) == 0)
                    flag = true;
            }
            if (!flag)
            {
                for (int k = 0; k < 100; k++)
                {
                    if (strlen(clients[i].msgs[k].sender_ip) == 0 && strlen(clients[i].msgs[k].msg) == 0)
                    {
                        strcpy(clients[i].msgs[k].msg, message);
                        strcpy(clients[i].msgs[k].sender_ip, sender_ip);
                        // cse4589_print_and_log("Added message\n");
                        break;
                    }
                }
            }
        }
    }
    if (send_atleast_once)
    {
        // Log the relayed broadcast message
        cse4589_print_and_log("[RELAYED:SUCCESS]\n");
        cse4589_print_and_log("msg from:%s, to:255.255.255.255\n[msg]:%s\n", sender_ip, message);
        cse4589_print_and_log("[RELAYED:END]\n");
        // cse4589_print_and_log("[BROADCAST:SUCCESS]\n[BROADCAST:END]\n");
    }
}

void start_server(int port)
{
    server_listening_port = port;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Create server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        // perror("Error: Socket creation failed");
        exit(1);
    }

    // Bind the socket
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        // perror("Error: Bind failed");
        close(server_socket);
        exit(1);
    }

    // Retrieve and store the server's IP address
    struct sockaddr_in stored_addr;
    socklen_t stored_len = sizeof(stored_addr);
    if (getsockname(server_socket, (struct sockaddr *)&stored_addr, &stored_len) == 0)
    {
        inet_ntop(AF_INET, &stored_addr.sin_addr, server_ip, INET_ADDRSTRLEN);
    }
    else
    {
        // perror("Error: Unable to retrieve server IP");
        strcpy(server_ip, "Unknown");
    }

    if (listen(server_socket, MAX_CLIENTS) < 0)
    {
        perror("Error: Listen failed");
        close(server_socket);
        exit(1);
    }

    // set default value for clients
    for (int j = 0; j < MAX_CLIENTS; j++)
    {
        clients[j].hostname[0] = '\0';   // Initialize with an empty string
        clients[j].is_logged_in = false; // Initialize login status
        for (int k = 0; k < MAX_CLIENTS; k++)
            strcpy(clients[j].blocked_ips[k], "");
    }

    FD_ZERO(&master_fds);
    FD_SET(server_socket, &master_fds);
    FD_SET(STDIN_FILENO, &master_fds);
    max_fd = server_socket;

    while (1)
    {
        read_fds = master_fds;

        // Use select() to monitor multiple connections and user input
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0)
        {
            // perror("Error: select() failed");
            exit(1);
        }

        // Iterate through all file descriptors
        for (int i = 0; i <= max_fd; i++)
        {
            if (FD_ISSET(i, &read_fds))
            {
                if (i == server_socket)
                {
                    // Accept new client connection
                    int new_client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
                    if (new_client_socket < 0)
                    {
                        // perror("Error: Accept failed");
                        continue;
                    }

                    // Add new client to fd_set
                    FD_SET(new_client_socket, &master_fds);
                    if (new_client_socket > max_fd)
                        max_fd = new_client_socket;

                    // Store client information
                    // loop over existing clients to update details for new and old client
                    // get hostname
                    char client_hostname[BUFFER_SIZE];
                    if (getnameinfo((struct sockaddr *)&client_addr, sizeof(client_addr), client_hostname, 1024, NULL, 0, 0) != 0)
                    {
                        strcpy(client_hostname, "Unknown");
                    }
                    // cse4589_print_and_log("client hostname: %s\n", client_hostname);
                    int i = 0;
                    bool match_found = false;
                    for (i = 0; i < MAX_CLIENTS; i++)
                    {
                        if (clients[i].hostname[0] != '\0' && strcmp(clients[i].hostname, client_hostname) == 0)
                        {
                            // match found, update the details
                            match_found = true;
                            clients[i].socket = new_client_socket;
                            clients[i].address = client_addr;
                            strcpy(clients[i].ipaddress, inet_ntoa(client_addr.sin_addr));
                            clients[i].port = ntohs(client_addr.sin_port);
                            clients[i].is_logged_in = true;
                            strcpy(clients[i].hostname, client_hostname);
                            // replay_msg(client_hostname);
                        }
                    }

                    if (!match_found)
                    {
                        while (client_count < MAX_CLIENTS)
                        {
                            if (!clients[client_count].is_logged_in)
                            {
                                clients[client_count].socket = new_client_socket;
                                clients[client_count].address = client_addr;
                                strcpy(clients[client_count].ipaddress, inet_ntoa(client_addr.sin_addr));
                                clients[client_count].port = ntohs(client_addr.sin_port);
                                clients[client_count].is_logged_in = true;
                                strcpy(clients[client_count].hostname, client_hostname);
                                memset(clients[client_count].msgs, 0, sizeof(clients[client_count].msgs));
                                client_count++;
                                break;
                            }
                        }
                    }

                    // udpate the active list of clients
                    build_active_clients_list();

                    send(new_client_socket, active_clients, client_count * sizeof(ActiveClientInfo), 0);

                    // cse4589_print_and_log("[LOGIN:SUCCESS]\n");

                    // cse4589_print_and_log("[LOGIN:END]\n");
                }
                else if (i == STDIN_FILENO)
                {
                    // Handle user input from the server
                    fgets(command, 1024, stdin);
                    command[strcspn(command, "\n")] = 0;

                    if (strcmp(command, "EXIT") == 0)
                    {
                        // Close all client connections
                        for (int j = 0; j <= max_fd; j++)
                        {
                            if (FD_ISSET(j, &master_fds) && j != server_socket)
                            {
                                close(j);
                            }
                        }
                        close(server_socket);
                        // cse4589_print_and_log("Server shutting down...\n");
                        exit(0);
                    }
                    else if (strcmp(command, "AUTHOR") == 0)
                    {
                        handle_author();
                    }
                    else if (strcmp(command, "IP") == 0)
                    {
                        handle_ip();
                    }
                    else if (strcmp(command, "PORT") == 0)
                    {
                        handle_port();
                    }
                    else if (strcmp(command, "LIST") == 0)
                    {
                        handle_list();
                    }
                    else if (strcmp(command, "STATISTICS") == 0)
                    {
                        handle_statistics();
                    }
                    else if (strncmp(command, "BLOCKED", 7) == 0)
                    {
                        char ip_to_block[IP_LENGTH];
                        memset(ip_to_block, 0, sizeof(ip_to_block));
                        struct sockaddr_in sa;
                        sscanf(command + 7, "%s", ip_to_block);
                        int result = inet_pton(AF_INET, ip_to_block, &(sa.sin_addr));
                        // cse4589_print_and_log("result : %d does ip exist %s\n", result,does_ip_exist(ip_to_block)?"true":"false" );
                        if (result == 1 && does_ip_exist(ip_to_block))
                        {
                            handle_get_blocked(command);
                        }
                        else
                        {
                            cse4589_print_and_log("[BLOCKED:ERROR]\n[BLOCKED:END]\n");
                        }
                    }
                }
                else
                {
                    // Handle client messages
                    char buffer[BUFFER_SIZE];
                    memset(buffer, 0, BUFFER_SIZE);
                    int bytes_received = recv(i, buffer, BUFFER_SIZE, 0);

                    // get client ip address
                    struct sockaddr_in client_addr;
                    socklen_t addr_len = sizeof(client_addr);
                    getpeername(i, (struct sockaddr *)&client_addr, &addr_len);

                    // get hostname
                    char client_ip[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);

                    char client_hostname[256];
                    if (getnameinfo((struct sockaddr *)&client_addr, addr_len, client_hostname, sizeof(client_hostname), NULL, 0, 0) == 0)
                    {
                        // cse4589_print_and_log("Client Hostname: %s\n", client_hostname);
                    }
                    else
                    {
                        strcpy(client_hostname, "Unknown");
                    }

                    if (bytes_received <= 0)
                    {
                        // on LOGOUT we need to mark the client as logged out
                        for (int i = 0; i < MAX_CLIENTS; i++)
                        {
                            if (strcmp(clients[i].hostname, client_hostname) == 0 && clients[i].is_logged_in)
                            {
                                clients[i].is_logged_in = false;
                            }
                        }
                        // cse4589_print_and_log("Client disconnected\n");
                        close(i);
                        FD_CLR(i, &master_fds);
                    }
                    else
                    {
                        // Handle SEND command inside the loop
                        if (strncmp(buffer, "SEND", 4) == 0)
                        {
                            int result = handle_send_command(client_ip, buffer);
                            for (int k = 0; k < MAX_CLIENTS && result == 1; k++)
                            {
                                if (strcmp(clients[k].hostname, client_hostname) == 0)
                                {
                                    clients[k].msg_sent++;
                                }
                            }
                            if(result){
                                send(i, "SEND SUCCESS", 12, 0);
                            }
                            else{
                                send(i, "SEND FAILED", 11, 0);
                            }
                        }
                        else if (strncmp(buffer, "BLOCK", 5) == 0)
                        {
                            char target_ip[INET_ADDRSTRLEN];
                            if (sscanf(buffer, "BLOCK %s", target_ip) != 1)
                            {
                                send(i, "BLOCK FAILED", 12, 0);
                            }
                            if (strcmp(target_ip, client_ip) == 0)
                            {
                                send(i, "BLOCK FAILED", 12, 0);
                                continue;
                            }
                            int status = handle_block_command(client_hostname, buffer);
                            // cse4589_print_and_log("%d", status);
                            if (status)
                            {
                                send(i, "BLOCK SUCCESS", 13, 0);
                            }
                            else
                            {
                                send(i, "BLOCK FAILED", 12, 0);
                            }
                        }
                        else if (strncmp(buffer, "UNBLOCK", 7) == 0)
                        {
                            int status = handle_unblock_command(client_hostname, buffer);
                            // cse4589_print_and_log("%d", status);
                            if (status)
                            {
                                send(i, "UNBLOCK SUCCESS", 15, 0);
                            }
                            else
                            {
                                send(i, "UNBLOCK FAILED", 14, 0);
                            }
                        }
                        else if (strncmp(buffer, "REFRESH", 7) == 0)
                        {
                            build_active_clients_list();
                            send(i, active_clients, client_count * sizeof(ActiveClientInfo), 0);
                        }
                        else if (strcmp(buffer, "EXIT") == 0)
                        {
                            // cse4589_print_and_log("client hostname %s\n", client_hostname);
                            for (int k = 0; k < MAX_CLIENTS; k++)
                            {
                                if (strcmp(clients[k].hostname, client_hostname) == 0)
                                {
                                    memset(&clients[k], 0, sizeof(ClientInfo));
                                }
                            }
                        }
                        else if (strncmp(buffer, "BROADCAST", 9) == 0)
                        {
                            handle_broadcast_command(client_ip, buffer);
                            for (int k = 0; k < MAX_CLIENTS; k++)
                            {
                                if (strcmp(clients[k].hostname, client_hostname) == 0)
                                {
                                    clients[k].msg_sent++;
                                }
                            }
                        }
                        else if (strcmp(buffer, "REPLAY") == 0)
                        {
                            replay_msg(client_hostname);
                            send(i, "SUCCESS", strlen("SUCCESS"), MSG_NOSIGNAL);
                        }
                    }
                }
            }
        }
    }
}
