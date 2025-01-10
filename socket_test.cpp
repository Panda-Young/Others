/***************************************************************************
 * Description: create a socket server for test
 * Date: 2024-05-09 19:49:23
 * version: 0.1.0
 * Author: Panda-Young
 * Copyright (c) 2024 by Panda-Young, All Rights Reserved.
 **************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define BUF_SIZE 1024
#define PORT_ID 2502
#define DEBUG_DATA 0

#ifdef __MINGW32__
#include <process.h>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

void ProcessClient(LPVOID lpParam)
{
    SOCKET clientSocket = (SOCKET)lpParam;
    char buffer[BUF_SIZE] = {0};

    // 接收并发送消息
    while (1) {
        memset(buffer, 0, BUF_SIZE);
        char *pChunkStart = buffer;
        int rev_size = recv(clientSocket, buffer, BUF_SIZE, 0);
        if (rev_size <= 0) {
            break;
        }
        printf("received %d Bytes.\n", rev_size);

#if DEBUG_DATA
        for (int i = 0; i < rev_size; i++) {
            printf("%02x ", buffer[i]);
            if ((i + 1) % 16 == 0) {
                printf("\n");
            }
        }
        printf("\n");
#endif
    }
    // 关闭连接
    closesocket(clientSocket);
    printf("close the socket id %d\n", clientSocket);
    printf("\nWaiting for connection...\n");
}

int main()
{
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    SOCKADDR_IN serverAddr, clientAddr;

    // 初始化Winsock
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // 创建socket
    serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    // 设置socket选项
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

    // 绑定socket到一个地址
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(PORT_ID);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    bind(serverSocket, (SOCKADDR *)&serverAddr, sizeof(SOCKADDR));

    // 开始监听连接
    listen(serverSocket, 20);
    printf("Waiting for connection...\n");

    // 循环接收新的连接
    int addrLen = sizeof(SOCKADDR);
    while (1) {
        clientSocket = accept(serverSocket, (SOCKADDR *)&clientAddr, &addrLen);
        printf("accept connection from socket id %d\n", clientSocket);
        _beginthread(ProcessClient, 0, (LPVOID)clientSocket);
    }

    // 清理Winsock
    WSACleanup();

    return 0;
}

#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

void ProcessClient(int clientSocket)
{
    char buffer[BUF_SIZE] = {0};

    // 接收并发送消息
    while (1) {
        memset(buffer, 0, BUF_SIZE);
        char *pChunkStart = buffer;
        int rev_size = recv(clientSocket, buffer, BUF_SIZE, 0);
        if (rev_size <= 0) {
            break;
        }
        printf("received %d Bytes.\n", rev_size);
#if DEBUG_DATA
        for (int i = 0; i < rev_size; i++) {
            printf("%02x ", buffer[i]);
            if ((i + 1) % 16 == 0) {
                printf("\n");
            }
        }
        printf("\n");
#endif
    }
    // 关闭连接
    close(clientSocket);
    printf("close the socket id %d\n", clientSocket);
    printf("\nWaiting for connection...\n");
}

int main()
{
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;

    // 创建socket
    serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    // 设置socket选项
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 绑定socket到一个地址
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(PORT_ID);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr));

    // 开始监听连接
    listen(serverSocket, 20);
    printf("Waiting for connection...\n");

    // 循环接收新的连接
    int addrLen = sizeof(struct sockaddr);
    while (1) {
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, (socklen_t *)&addrLen);
        printf("accept connection from socket id %d\n", clientSocket);
        ProcessClient(clientSocket);
    }

    return 0;
}
#endif
