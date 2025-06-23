/* **************************************************************************
 * @Description: 
 * @Version: 0.1.0
 * @Author: pandapan@aactechnologies.com
 * @Date: 2025-06-20 15:40:21
 * @Copyright (c) 2025 by @AAC Technologies, All Rights Reserved.
 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 2501
#define BUF_SIZE 1024

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUF_SIZE];
    socklen_t addr_len = sizeof(client_addr);

    printf("[DEBUG] Creating socket...\n");
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("[ERROR] socket failed");
        exit(EXIT_FAILURE);
    }

    printf("[DEBUG] Setting server address structure...\n");
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // 监听所有IP
    server_addr.sin_port = htons(PORT);

    printf("[DEBUG] Binding socket to port %d...\n", PORT);
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("[ERROR] bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("[DEBUG] Listening for connections...\n");
    if (listen(server_fd, 3) == -1) {
        perror("[ERROR] listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("[INFO] Server listening on port %d ...\n", PORT);

    printf("[DEBUG] Waiting for client connection...\n");
    client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
    if (client_fd == -1) {
        perror("[ERROR] accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("[INFO] Client connected: %s:%d\n",
        inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // 收发消息
    while (1) {
        memset(buffer, 0, BUF_SIZE);
        printf("[DEBUG] Waiting to receive data from client...\n");
        int recv_len = recv(client_fd, buffer, BUF_SIZE, 0);
        if (recv_len <= 0) {
            printf("[INFO] Client disconnected or recv failed (recv_len=%d).\n", recv_len);
            break;
        }
        printf("[INFO] Received (%d bytes): %s\n", recv_len, buffer);

        // 回送消息
        printf("[DEBUG] Sending data back to client...\n");
        int sent_len = send(client_fd, buffer, recv_len, 0);
        printf("[DEBUG] Sent %d bytes back to client.\n", sent_len);
    }

    printf("[DEBUG] Closing sockets...\n");
    close(client_fd);
    close(server_fd);
    printf("[INFO] Server exited.\n");
    return 0;
}
