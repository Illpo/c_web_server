#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "server_start.c"
#include "files.c"

#define MAX_CLIENTS 100
#define MAX_BUFFER_SIZE 2024


int main() {

    SOCKET serverSocket = ServerStart();

    fd_set readfds;
    SOCKET clients[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i] = INVALID_SOCKET;
    }

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(serverSocket, &readfds);
        int max_sd = serverSocket;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] != INVALID_SOCKET) {
                FD_SET(clients[i], &readfds);
            }
            if (clients[MAX_CLIENTS] > max_sd) {
                max_sd = clients[MAX_CLIENTS];
            }
        }

        if (select(serverSocket + 1, &readfds, NULL, NULL, NULL) == SOCKET_ERROR) {
            printf("Select failed.\n");
            cleanupAndExit(serverSocket);
        }

        if (FD_ISSET(serverSocket, &readfds)) {
            struct sockaddr_in clientAddr;
            int clientAddrLen = sizeof(clientAddr);
            SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
            if (clientSocket == INVALID_SOCKET) {
                printf("Accept failed.\n");
                continue;
            } else {
                printf("New connection accepted.\nIP: %s\n", inet_ntoa(clientAddr.sin_addr));
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (clients[i] == INVALID_SOCKET) {
                        clients[i] = clientSocket;
                        break;
                    }
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] != INVALID_SOCKET && FD_ISSET(clients[i], &readfds)) {
                char buff[MAX_BUFFER_SIZE];
                memset(buff, 0, sizeof(buff));
                int bytesRead = recv(clients[i], buff, sizeof(buff), 0);
                if (bytesRead > 0) {

                    char *method, *path;

                    method = (char *)malloc(4 * sizeof(char));
                    path = (char *)malloc(11 * sizeof(char));

                    sscanf(buff, "%s %s", method, path);

                    printf("%s\n\n", buff);

                    if (strcmp(method, "GET") == 0) {

                        if(strcmp(path, "/") == 0) {
                            handleGETRequest(clients[i], "index.html");
                            closesocket(clients[i]);
                            clients[i] = INVALID_SOCKET;
                            free(method);
                            free(path);
                        } else {
                            path++;
                          
                            if(strncmp(path, "static", strlen("static")) == 0 || strcmp(path, "favicon.ico") == 0) {

                                handleGETRequest_forStatic(clients[i], path);
                                closesocket(clients[i]);
                                clients[i] = INVALID_SOCKET;
                                free(method);
                                free(path);

                            } else {
                                const char* notImplementedResponse = "HTTP/1.1 505 No\r\n";
                                send(clients[i], notImplementedResponse, strlen(notImplementedResponse), 0);
                                printf("%s\n", notImplementedResponse);
                                closesocket(clients[i]);
                                clients[i] = INVALID_SOCKET;
                                free(method);
                                free(path);
                            }
                        }

                    } else if (strcmp(method, "POST") == 0) {

                        if(strcmp(path, "/register") == 0) {
                            char *tok = strtok(buff, "\n");
                            char *last = NULL;
                            while(tok != NULL) {
                                last = tok;
                                tok = strtok(NULL, "\n");
                            }

                            printf("Data: %s\n", last);
                            redirect(clients[i]);
                            free(method);
                            free(path);

                        } else {
                            // Return 404 for unknown POST paths
                            const char* notFoundResponse = "HTTP/1.1 404 Not Found\r\n\r\n";
                            send(clients[i], notFoundResponse, strlen(notFoundResponse), 0);
                            closesocket(clients[i]);
                            clients[i] = INVALID_SOCKET;
                            free(method);
                            free(path);
                        }                        
                    } else {
                        const char* notImplementedResponse = "HTTP/1.1 501 Not Implemented\r\n\r\n";
                        send(clients[i], notImplementedResponse, strlen(notImplementedResponse), 0);
                        closesocket(clients[i]);
                        clients[i] = INVALID_SOCKET;
                        free(method);
                        free(path);
                    }
                } else {
                    printf("Recv failed.\n");
                    closesocket(clients[i]);
                    clients[i] = INVALID_SOCKET;
                }
            }
        }
    }

    cleanupAndExit(serverSocket);
    return EXIT_SUCCESS;
}
