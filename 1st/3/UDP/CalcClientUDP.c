#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <netdb.h>
#include <poll.h>
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
        exit(1);
    }

    const char *hostname = argv[1];
    int port = atoi(argv[2]);

    struct hostent *host = gethostbyname(hostname);
    if (host == NULL)
    {
        perror("Hostname resolution failed");
        exit(1);
    }

    int clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    // if (clientSocket == -1) {
    //     perror("Socket creation failed");
    //     exit(1);
    // }
    if (clientSocket == -1)
    {
        perror("Socket creation failed");
        return 1;
    }
    // Set up timeout
struct timeval timeout;
timeout.tv_sec = 5;  // Adjust as needed
timeout.tv_usec = 0;

// Set up the file descriptor set for select
fd_set readfds;
FD_ZERO(&readfds);
FD_SET(clientSocket, &readfds);

// Use select to wait for socket activity or timeout
int selectResult = select(clientSocket + 1, &readfds, NULL, NULL, &timeout);
if (selectResult == -1) {
    perror("select");
    close(clientSocket);
    return 1;
} else if (selectResult == 0) {
    printf("No response from server within %ld seconds.\n", timeout.tv_sec);
    close(clientSocket);
    return 1;
}
    // struct pollfd ufds[1];
    // ufds[0].fd = clientSocket;
    // ufds[0].events =POLLIN;

    // int pollResult = poll(ufds, 1, timeout);
    // printf("%d\n",pollResult);
// if (pollResult == -1) {
//     perror("poll");
//     close(clientSocket);
//     return 1;
// } else if (pollResult == 0) {
//     printf("No response from server within %d seconds.\n", timeout / 1000);
//     close(clientSocket);
//     return 1;
// }


    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    memcpy(&serverAddr.sin_addr, host->h_addr_list[0], host->h_length);

    // Print connected hostname and port
    printf("Connected to %s on port %d\n", hostname, port);

    char expression[256];
    while (1)
    {
        printf("Enter an expression (or -1 to quit): ");
        fgets(expression, sizeof(expression), stdin);

        size_t expressionLength = strlen(expression);

        if (expressionLength > 0 && expression[expressionLength - 1] == '\n')
        {
            expression[expressionLength - 1] = '\0';
        }

        if (strcmp(expression, "-1") == 0)
        {
            sendto(clientSocket, expression, strlen(expression), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
            printf("Bye!\n");
            break;
        }

        sendto(clientSocket, expression, strlen(expression), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

        char result[50];
        socklen_t serverAddrLen = sizeof(serverAddr);
        int bytesReceived = recvfrom(clientSocket, result, sizeof(result), 0, (struct sockaddr *)&serverAddr, &serverAddrLen);

        if (bytesReceived > 0)
        {
            result[bytesReceived] = '\0';
            printf("Received from server: %s\n", result);
        }
    }

    close(clientSocket);
    return 0;
}