#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr;

    // Create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        perror("Error creating socket");
        return 1;
    }

    // Setup server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);  // Use the same port as the server
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  // Use the server's IP address

    // Connect
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Error connecting to server");
        return 1;
    }

    printf("Connected to server.\n");

    while (1) {
        printf("\033[0;36m");  // Set color to cyan
        printf("Client: ");
        printf("\033[0m");     // Reset color
        char buffer[1024];
        fgets(buffer, sizeof(buffer), stdin);

        // Handle graceful exit command
        if (strcmp(buffer, "/exit\n") == 0) {
            printf("Exiting...\n");
            send(clientSocket, buffer, strlen(buffer), 0);
            break;
        }

        send(clientSocket, buffer, strlen(buffer), 0);

        // Receive reply from server
        ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            printf("Server disconnected.\n");
            break;
        }
        
        buffer[bytesReceived] = '\0';

        printf("\033[0;33m");  // Set color to yellow
        printf("Server: %s", buffer);
        printf("\033[0m");     // Reset color
    }

    close(clientSocket);

    return 0;
}
