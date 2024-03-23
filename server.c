#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <wchar.h>
#include <fcntl.h>
#include <io.h>

#include "server_start.c"
#include "files.c"

#define MAX_CLIENTS 100
#define MAX_BUFFER_SIZE 4048
#define _O_U16TEXT 0x20000
#define _O_U8TEXT 0x00040000

int main() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    int result = _setmode( _fileno( stdout ), _O_U8TEXT );
    if( result == -1 ) {
      perror( "Cannot set mode" );
    }
    wprintf(L"┌──Server running...\n└─» HELLO ");
    SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
    wprintf(L"Mr. ILLPO \n");
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    wprintf(L" ┌───┐  ┌───┐\n | ▀▄▀  ▀▄▀ |\n └───┘  └───┘\n");
    if (_setmode(_fileno( stdout ), result) == -1) {
        perror("_setmode");
        return 1;
    }
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
                if (clients[i] > max_sd) { 
                    max_sd = clients[i]; 
                }
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
                    char *secret_word = "/helloworld";
                    char *method = NULL;
                    char *path = NULL;

                    char *tokens[30] = {NULL};
            
                    printf("%s\n", buff);
                    char *token = strtok(buff, " ");
                    int c=0;
                    while(token != NULL && c < 30) {
                        tokens[c++] = token;
                        token = strtok(NULL, " ");
                    }
                    if(tokens[0] != NULL && tokens[1] != NULL) {
                        method = tokens[0];
                        path = tokens[1];
                    }

                    if (strcmp(method, "GET") == 0) {

                        if(strcmp(path, "/") == 0) {
                            handleGETRequest(clients[i], "index.html");
                            closesocket(clients[i]);
                            clients[i] = INVALID_SOCKET;
                        } else if(strcmp(path, "/no") ==0) {
                            handleGETRequest(clients[i], "no.html");
                            closesocket(clients[i]);
                            clients[i] = INVALID_SOCKET;
                        } else if(strcmp(path, secret_word) ==0) {
                            closesocket(clients[i]);
                            clients[i] = INVALID_SOCKET;
                            cleanupAndExit(serverSocket);
                            return EXIT_SUCCESS;
                        } else {
                            path++;
                          
                            if(strncmp(path, "static", strlen("static")) == 0 || strcmp(path, "favicon.ico") == 0) {

                                handleGETRequest_forStatic(clients[i], path);
                                closesocket(clients[i]);
                                clients[i] = INVALID_SOCKET;
                                method = path = NULL;

                            } else {
                                const char* notImplementedResponse = "HTTP/1.1 505 No\r\n";
                                send(clients[i], notImplementedResponse, strlen(notImplementedResponse), 0);
                                printf("%s\n", notImplementedResponse);
                                closesocket(clients[i]);
                                clients[i] = INVALID_SOCKET;
                            }
                        }

                    } else if (strcmp(method, "POST") == 0) {

                        if(strcmp(path, "/register") == 0) {

                            redirect(clients[i]);

                        } else {
                            
                            const char* notFoundResponse = "HTTP/1.1 404 Not Found\r\n\r\n";
                            send(clients[i], notFoundResponse, strlen(notFoundResponse), 0);
                            closesocket(clients[i]);
                            clients[i] = INVALID_SOCKET;
                        }                        
                    } else {
                        const char* notImplementedResponse = "HTTP/1.1 501 Not Implemented\r\n\r\n";
                        send(clients[i], notImplementedResponse, strlen(notImplementedResponse), 0);
                        closesocket(clients[i]);
                        clients[i] = INVALID_SOCKET;
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
