#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

#define MAX_EXPRESSION_SIZE 256
double evaluateExpression(const char *expression) {
    double operand1, operand2;
    char operator;
    
    if (sscanf(expression, "%lf %c %lf", &operand1, &operator, &operand2) != 3) {
        printf("Invalid expression format\n");
        return 0.0;
    }

    double result = 0.0;

    switch (operator) {
        case '+':
            result = operand1 + operand2;
            break;
        case '-':
            result = operand1 - operand2;
            break;
        case '*':
            result = operand1 * operand2;
            break;
        case '/':
            if (operand2 != 0) {
                result = operand1 / operand2;
            } else {
                printf("Division by zero is not allowed\n");
                return 0.0;
            }
            break;
        case '^':
            result = pow(operand1, operand2);
            break;
        default:
            printf("Invalid operator\n");
            return 0.0;
    }

    return result;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);

    int serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Binding failed");
        close(serverSocket);
        exit(1);
    }

    printf("Server listening on port %d\n", port);

    while (1) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);

        char expression[MAX_EXPRESSION_SIZE];
        int bytesReceived = recvfrom(serverSocket, expression, sizeof(expression), 0, (struct sockaddr *)&clientAddr, &clientAddrLen);

        if (bytesReceived <= 0) {
            // Error or client closed connection
            printf("Client closed connection\n");
            continue;
        }

        expression[bytesReceived] = '\0';

        if (strcmp(expression, "-1") == 0) {
            // Client wants to quit
            printf("Client closed connection\n");
            continue;
        }

        // Print the received expression
        printf("Received from client %s:%d: %s\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), expression);

        // Evaluate the expression
        double result = evaluateExpression(expression);

        char resultStr[50];
        snprintf(resultStr, sizeof(resultStr), "%.2f", result);

        // Send result back to client
        sendto(serverSocket, resultStr, strlen(resultStr), 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));

        // Print the answer that is being sent
        printf("Sending to client %s:%d: %s\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), resultStr);
    }

    close(serverSocket);
    return 0;
}