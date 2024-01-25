#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 82

void cleanupAndExit(SOCKET serverSocket) {
    closesocket(serverSocket);
    WSACleanup();
    exit(EXIT_FAILURE);
}

SOCKET ServerStart() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Failed to initialize Winsock.\n");
        return EXIT_FAILURE;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        printf("Failed to create socket.\n");
        cleanupAndExit(serverSocket);
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("Your address");
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Bind failed.\n");
        cleanupAndExit(serverSocket);
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("Listen failed.\n");
        cleanupAndExit(serverSocket);
    }

    printf("Server listening on port %d...\n", PORT);

    return serverSocket;
}