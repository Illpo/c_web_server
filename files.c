#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUFFER_SIZE 2024

const char* get_type(const char* file_extension) {
    if (strcmp(file_extension, ".html") == 0) {
        return "text/html";
    } else if (strcmp(file_extension, ".css") == 0) {
        return "text/css";
    } else if (strcmp(file_extension, ".jpg") == 0 || strcmp(file_extension, ".jpeg") == 0) {
        return "image/jpeg";
    } else if (strcmp(file_extension, ".png") == 0) {
        return "image/png";
    } else {
        return "application/octet-stream";
    }
}

void handleGETRequest(SOCKET client, const char* path) {
    
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        const char* notFoundResponse = "HTTP/1.1 404 Not Found\r\n\r\n";
        send(client, notFoundResponse, strlen(notFoundResponse), 0);
        return;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    const char* type = get_type(strrchr(path, '.'));
    char responseHeader[256];
    sprintf(responseHeader, "HTTP/1.1 200 OK\r\nServer: CIA/FBI\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n", type, fileSize);
    send(client, responseHeader, strlen(responseHeader), 0);
    printf("%s\n", responseHeader);
    char fileBuffer[MAX_BUFFER_SIZE];
    size_t bytesRead = 0;
    while ((bytesRead = fread(fileBuffer, 1, MAX_BUFFER_SIZE, file)) > 0) {
        send(client, fileBuffer, bytesRead, 0);
    }

    fclose(file);
}

void handleGETRequest_forStatic(SOCKET client, char* path) {
    
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        const char* notFoundResponse = "HTTP/1.1 404 Not Found\r\n\r\n";
        send(client, notFoundResponse, strlen(notFoundResponse), 0);
        printf("%s\n", notFoundResponse);
        return;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    const char* type = get_type(strrchr(path, '.'));
    char responseHeader[256];
    sprintf(responseHeader, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nCache-Control: max-age=2\r\nContent-Length: %d\r\n\r\n", type, fileSize);
    send(client, responseHeader, strlen(responseHeader), 0);
    printf("%s\n", responseHeader);
    char fileBuffer[MAX_BUFFER_SIZE];
    size_t bytesRead = 0;
    while ((bytesRead = fread(fileBuffer, 1, sizeof(fileBuffer), file)) > 0) {
        send(client, fileBuffer, bytesRead, 0);
    }

    fclose(file);
}

void redirect(SOCKET client) {

    char responseHeader[256];
    sprintf(responseHeader, "HTTP/1.1 301 Move\r\nLocation: /\r\n\r\n");
    send(client, responseHeader, strlen(responseHeader), 0);
    printf("%s\n", responseHeader);

}