#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

void* handleClient(void* clientSocketPtr) {
    int clientSocket = *((int*)clientSocketPtr);
    char buffer[1024];

    while (1) {
        ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            printf("Client %d disconnected.\n", clientSocket);
            break;
        }

        buffer[bytesReceived] = '\0';

        // Handle graceful exit command
        if (strcmp(buffer, "/exit\n") == 0) {
            printf("Client %d requested exit.\n", clientSocket);
            send(clientSocket, "Server: Goodbye!\n", 18, 0); // Send farewell message
            break;
        }

        printf("Client %d: %s", clientSocket, buffer);

        // Send reply to client
        printf("Server: ");
        fgets(buffer, sizeof(buffer), stdin);
        send(clientSocket, buffer, strlen(buffer), 0);
    }

    close(clientSocket);
    free(clientSocketPtr);
    return NULL;
}

int main() {
    int serverSocket;
    struct sockaddr_in serverAddr;

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Error creating socket");
        return 1;
    }

    // Setup server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);  // Use any available port
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Error binding");
        return 1;
    }

    // Listen
    if (listen(serverSocket, 5) == -1) {
        perror("Error listening");
        return 1;
    }

    printf("Server is listening for incoming connections.\n");

    while (1) {
        // Accept connection
        int* clientSocketPtr = (int*)malloc(sizeof(int));
        *clientSocketPtr = accept(serverSocket, NULL, NULL);
        if (*clientSocketPtr == -1) {
            perror("Error accepting client connection");
            continue;
        }

        printf("Client connected.\n");

        pthread_t clientThread;
        if (pthread_create(&clientThread, NULL, handleClient, clientSocketPtr) != 0) {
            perror("Error creating thread");
            close(*clientSocketPtr);
            free(clientSocketPtr);
        }
    }

    close(serverSocket);

    return 0;
}
