
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/select.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
        exit(1);
    }

    const char *hostname = argv[1];
    int port = atoi(argv[2]);

    struct hostent *host = gethostbyname(hostname);
    if (host == NULL) {
        perror("Hostname resolution failed");
        exit(1);
    }

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    memcpy(&serverAddr.sin_addr, host->h_addr_list[0], host->h_length);

    // Connect to server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Connection failed");
        close(clientSocket);
        printf("Server is not online.\n");
        return 1;
    }

    printf("Connected to %s:%d\n", inet_ntoa(serverAddr.sin_addr), port);

    char expression[256];
    while (1) {
        printf("Enter an expression (or -1 to quit): ");
        fgets(expression, sizeof(expression), stdin);

        size_t expressionLength = strlen(expression);

        if (expressionLength > 0 && expression[expressionLength - 1] == '\n') {
            expression[expressionLength - 1] = '\0';
        }

        if (strcmp(expression, "-1") == 0) {
            break;
        }

        send(clientSocket, expression, strlen(expression), 0);

        char result[50];
        int bytesReceived = recv(clientSocket, result, sizeof(result), 0);

        if (bytesReceived > 0) {
            result[bytesReceived] = '\0';
            printf("Received from server: %s\n", result);
        }
    }

    close(clientSocket);
    return 0;
}