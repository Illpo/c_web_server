#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


#include "server_start.c"
#include "files.c"
#include "terminal.c"

#define MAX_CLIENTS 100
#define MAX_BUFFER_SIZE 4096
//#define _O_U16TEXT 0x20000
//#define _O_U8TEXT 0x00040000

int main() {
    if(!t_start()) {
        printf("error with terminal\n");
        return 0;
    } 
    SOCKET serverSocket = ServerStart();

    fd_set readfds;
    SOCKET clients[MAX_CLIENTS];
    for (uint8_t i = 0; i < MAX_CLIENTS; i++) {
        clients[i] = INVALID_SOCKET;
    }

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(serverSocket, &readfds);
        int max_sd = serverSocket;

        for (uint8_t i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] != INVALID_SOCKET) {
                FD_SET(clients[i], &readfds);
                if (clients[i] > max_sd) { 
                    max_sd = clients[i]; 
                }
            }
        }

        if (select(max_sd + 1, &readfds, NULL, NULL, NULL) == SOCKET_ERROR) {
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
                char ipStr[16];
                printf("New connection accepted.\nIP: %s..:\n", inet_ntop(AF_INET, &(clientAddr.sin_addr), ipStr, 16));
                memset(ipStr, 0, sizeof(ipStr));
                for (uint8_t i = 0; i < MAX_CLIENTS; i++) {
                    if (clients[i] == INVALID_SOCKET) {
                        clients[i] = clientSocket;
                        break;
                    }
                }
            }
        }

        for (uint8_t i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] != INVALID_SOCKET && FD_ISSET(clients[i], &readfds)) {
                char *buff = (char*)malloc(MAX_BUFFER_SIZE);
                //memset(buff, 0, sizeof(buff));
                uint16_t bytesRead = recv(clients[i], buff, MAX_BUFFER_SIZE-1, 0);
                if (bytesRead > 0) {

                    buff[bytesRead] = '\0';
                    char *secret_word = "/helloworld";
                    printf("%s\n", buff);
            
                    char *tokens[30] = {NULL};
                    char *context = NULL;
                    char *token = strtok_s(buff, " ", &context);

                    uint8_t c=0;
                    while(token != NULL && c < 30) {
                        tokens[c++] = token;
                        token = strtok_s(NULL, " ", &context);
                    }
                    
                    char *method = tokens[0] ? tokens[0] : NULL;
                    char *path = tokens[1] ? tokens[1] : NULL;

                    if (strcmp(method, "GET") == 0) {

                        if(strcmp(path, "/") == 0) {            handleGETRequest(clients[i], "index.html"); }
                        else if(strcmp(path, "/no") ==0) {      handleGETRequest(clients[i], "no.html");    }
                        else if(strcmp(path, "/no2") ==0) {     handleGETRequest(clients[i], "no2.html");   }
                        else if(strcmp(path, "/login") ==0) {   handleGETRequest(clients[i], "login.html"); }
                        else if(strcmp(path, secret_word) ==0) {
                            closesocket(clients[i]);
                            clients[i] = INVALID_SOCKET;
                            cleanupAndExit(serverSocket);
                            free(buff);
                            buff = NULL;
                            return EXIT_SUCCESS;
                        } else {
                            path++;
                            if(strncmp(path, "static", strlen("static")) == 0 || strcmp(path, "favicon.ico") == 0) {

                                handleGETRequest_forStatic(clients[i], path);
                                
                            } else {
                                const char* notImplementedResponse = "HTTP/1.1 505 No\r\n";
                                send(clients[i], notImplementedResponse, strlen(notImplementedResponse), 0);
                                printf("%s\n", notImplementedResponse);
  
                            }
                        }
                        
                    } else if (strcmp(method, "POST") == 0) {

                        if(strcmp(path, "/register") == 0) {

                            redirect(clients[i]);

                        } else {
                            
                            const char* notFoundResponse = "HTTP/1.1 404 Not Found\r\n\r\n";
                            send(clients[i], notFoundResponse, strlen(notFoundResponse), 0);

                        } 

                    } else {
                        const char* notImplementedResponse = "HTTP/1.1 501 Not Implemented\r\n\r\n";
                        send(clients[i], notImplementedResponse, strlen(notImplementedResponse), 0);

                    }

                    closesocket(clients[i]);
                    clients[i] = INVALID_SOCKET;
                    free(buff);
                    buff = NULL;
                    
                } else {
                    printf("Recv failed.\n");
                    closesocket(clients[i]);
                    clients[i] = INVALID_SOCKET;

                    free(buff);
                    buff = NULL;

                }
            }
        }
    }

    cleanupAndExit(serverSocket);
    return EXIT_SUCCESS;
}
