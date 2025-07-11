/***************************************************************************
 * Description: create a socket client for test
 * Date: 2024-05-09 20:00:00
 * version: 0.1.0
 * Author: Panda-Young
 * Copyright (c) 2024 by Panda-Young, All Rights Reserved.
 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 1024
#define SERVER_IP "127.0.0.1"
#define PORT_ID 2502

#ifdef __MINGW32__
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

int main()
{
    WSADATA wsaData;
    SOCKET clientSocket;
    SOCKADDR_IN serverAddr;
    char buffer[BUF_SIZE] = "Hello from client";

    // 初始化Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed.\n");
        return 1;
    }

    // 创建socket
    clientSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        printf("Socket creation failed.\n");
        WSACleanup();
        return 1;
    }

    // 设置服务器地址
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(PORT_ID);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // 连接到服务器
    if (connect(clientSocket, (SOCKADDR *)&serverAddr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
        printf("Connection to server failed.\n");
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    // 发送数据
    send(clientSocket, buffer, strlen(buffer), 0);
    printf("Sent: %s\n", buffer);
    printf("Sent %lu Bytes.\n", strlen(buffer));

    // 关闭连接
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}

#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main()
{
    int clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[BUF_SIZE] = "Hello from client";

    // 创建socket
    clientSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket < 0) {
        printf("Socket creation failed.\n");
        return 1;
    }

    // 设置服务器地址
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(PORT_ID);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // 连接到服务器
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr)) < 0) {
        printf("Connection to server failed.\n");
        close(clientSocket);
        return 1;
    }

    // 发送数据
    send(clientSocket, buffer, strlen(buffer), 0);
    printf("Sent: %s\n", buffer);
    printf("Sent %lu Bytes.\n", strlen(buffer));

    // 关闭连接
    close(clientSocket);

    return 0;
}
#endif
