/* **************************************************************************
 * @Description: Windows Socket Client Implementation
 * @Version: 1.0.0
 * @Author: pandapan@aactechnologies.com
 * @Date: 2025-06-20 15:40:31
 * @Copyright (c) 2025 by @AAC Technologies, All Rights Reserved.
 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "192.168.0.105" // Replace with actual server IP
#define PORT 2501
#define BUF_SIZE 1024

int main()
{
    WSADATA wsaData;
    printf("[DEBUG] Initializing Winsock...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("[ERROR] WSAStartup failed.\n");
        return 1;
    }

    SOCKET sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUF_SIZE];

    printf("[DEBUG] Creating socket...\n");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        perror("[ERROR] socket failed");
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    printf("[DEBUG] Setting server address structure...\n");
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    printf("[DEBUG] Connecting to server %s:%d ...\n", SERVER_IP, PORT);
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        perror("[ERROR] connect failed");
        closesocket(sockfd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    printf("[INFO] Connected to server %s:%d\n", SERVER_IP, PORT);

    // Set receive timeout (5 seconds)
    DWORD timeout = 5000;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));

    // Message loop
    while (1) {
        printf("Enter message (or 'exit' to quit): ");
        fgets(buffer, BUF_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0; // Remove newline

        if (strcmp(buffer, "exit") == 0)
            break;

        printf("[DEBUG] Sending data to server...\n");
        int sent_len = send(sockfd, buffer, strlen(buffer), 0);
        printf("[DEBUG] Sent %d bytes to server.\n", sent_len);

        memset(buffer, 0, BUF_SIZE);
        printf("[DEBUG] Waiting to receive data from server...\n");
        int recv_len = recv(sockfd, buffer, BUF_SIZE, 0);

        if (recv_len <= 0) {
            if (WSAGetLastError() == WSAETIMEDOUT) {
                printf("[WARN] Receive timeout! Server not responding.\n");
            } else {
                printf("[ERROR] recv failed: %d\n", WSAGetLastError());
            }
            continue;
        }
        printf("[INFO] Received from server (%d bytes): %s\n", recv_len, buffer);
    }

    closesocket(sockfd);
    WSACleanup();
    printf("[INFO] Client exited.\n");
    return 0;
}
