// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <arpa/inet.h>

// int calculate(char *expression) {
//     // Implement your calculation logic here
//     // You'll need to parse the expression and perform the calculations
//     // Return the result
// }

// int main(int argc, char *argv[]) {
//     if (argc != 2) {
//         fprintf(stderr, "Usage: %s <port>\n", argv[0]);
//         return 1;
//     }

//     int serverPort = atoi(argv[1]);

//     int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
//     if (serverSocket < 0) {
//         perror("Socket creation failed");
//         return 1;
//     }

//     struct sockaddr_in serverAddr;
//     serverAddr.sin_family = AF_INET;
//     serverAddr.sin_addr.s_addr = INADDR_ANY;
//     serverAddr.sin_port = htons(serverPort);

//     if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
//         perror("Binding failed");
//         return 1;
//     }

//     if (listen(serverSocket, 5) < 0) {
//         perror("Listening failed");
//         return 1;
//     }

//     printf("Server listening on port %d\n", serverPort);

//     while (1) {
//         int clientSocket;
//         struct sockaddr_in clientAddr;
//         socklen_t clientAddrLen = sizeof(clientAddr);

//         clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
//         if (clientSocket < 0) {
//             perror("Accepting connection failed");
//             continue;
//         }

//         char clientIP[INET_ADDRSTRLEN];
//         inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
//         printf("TCP client connected from %s on port %d\n", clientIP, ntohs(clientAddr.sin_port));

//         char expression[100];
//         while (1) {
//             recv(clientSocket, expression, sizeof(expression), 0);
//             if (strcmp(expression, "-1") == 0) {
//                 printf("Client closed connection\n");
//                 close(clientSocket);
//                 break;
//             }

//             int result = calculate(expression);
//             char resultMessage[100];
//             snprintf(resultMessage, sizeof(resultMessage), "ANS: %s = %d", expression, result);

//             send(clientSocket, resultMessage, strlen(resultMessage), 0);
//         }
//     }

//     close(serverSocket);
//     return 0;
// }
// #include <stdio.h>
// #include <stdlib.h>
// #include <math.h>

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <arpa/inet.h>
// #include <unistd.h>
// #include <sys/socket.h>

// #define MAX_EXPRESSION_SIZE 256


// double evaluateExpression(const char *expression) {
//     double operand1, operand2;
//     char operator;
    
//     if (sscanf(expression, "%lf %c %lf", &operand1, &operator, &operand2) != 3) {
//         printf("Invalid expression format\n");
//         return 0.0;
//     }

//     double result = 0.0;

//     switch (operator) {
//         case '+':
//             result = operand1 + operand2;
//             break;
//         case '-':
//             result = operand1 - operand2;
//             break;
//         case '*':
//             result = operand1 * operand2;
//             break;
//         case '/':
//             if (operand2 != 0) {
//                 result = operand1 / operand2;
//             } else {
//                 printf("Division by zero is not allowed\n");
//                 return 0.0;
//             }
//             break;
//         case '^':
//             result = pow(operand1, operand2);
//             break;
//         default:
//             printf("Invalid operator\n");
//             return 0.0;
//     }

//     return result;
// }

// int main(int argc, char *argv[]) {
//     if (argc != 2) {
//         fprintf(stderr, "Usage: %s <port>\n", argv[0]);
//         exit(1);
//     }

//     int port = atoi(argv[1]);

//     // Create socket
//     int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
//     if (serverSocket == -1) {
//         perror("Socket creation failed");
//         exit(1);
//     }

//     struct sockaddr_in serverAddr;
//     serverAddr.sin_family = AF_INET;
//     serverAddr.sin_port = htons(port);
//     serverAddr.sin_addr.s_addr = INADDR_ANY;

//     // Bind
//     if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
//         perror("Binding failed");
//         close(serverSocket);
//         exit(1);
//     }

//     // Listen
//     if (listen(serverSocket, 5) == -1) {
//         perror("Listening failed");
//         close(serverSocket);
//         exit(1);
//     }

//     // while (1) {
//     //     struct sockaddr_in clientAddr;
//     //     socklen_t clientAddrLen = sizeof(clientAddr);

//     //     // Accept connection
//     //     int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
//     //     if (clientSocket == -1) {
//     //         perror("Accepting connection failed");
//     //         continue;
//     //     }

//     //     // Print client information
//     //     printf("TCP client connected from %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

//     //     char expression[MAX_EXPRESSION_SIZE];
//     //     int bytesReceived = recv(clientSocket, expression, sizeof(expression), 0);

//     //     if (bytesReceived > 0) {
//     //         expression[bytesReceived] = '\0';
//     //         double result = evaluateExpression(expression);
//     //         char resultStr[50];
//     //         snprintf(resultStr, sizeof(resultStr), "%.2f", result);

//     //         // Send result back to client
//     //         send(clientSocket, resultStr, strlen(resultStr), 0);
//     //     }

//     //     close(clientSocket);
//     // }
//     while (1) {
//     struct sockaddr_in clientAddr;
//     socklen_t clientAddrLen = sizeof(clientAddr);

//     // Accept connection
//     int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
//     if (clientSocket == -1) {
//         perror("Accepting connection failed");
//         continue;
//     }

//     // Print client information
//     printf("TCP client connected from %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

//     char expression[MAX_EXPRESSION_SIZE];
//     int bytesReceived = recv(clientSocket, expression, sizeof(expression), 0);

//     if (bytesReceived > 0) {
//         expression[bytesReceived] = '\0';
//         double result = evaluateExpression(expression);
//         char resultStr[50];
//         snprintf(resultStr, sizeof(resultStr), "%.2f", result);

//         // Send result back to client
//         send(clientSocket, resultStr, strlen(resultStr), 0);
//     }

//     close(clientSocket);  // Close the client socket
// }
//     close(serverSocket);
//     return 0;
// }



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
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

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
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

    if (listen(serverSocket, 5) == -1) {
        perror("Listening failed");
        close(serverSocket);
        exit(1);
    }

    // while (1) {
    //     struct sockaddr_in clientAddr;
    //     socklen_t clientAddrLen = sizeof(clientAddr);

    //     int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
    //     if (clientSocket == -1) {
    //         perror("Accepting connection failed");
    //         continue;
    //     }

    //     // Print client information
    //     printf("TCP client connected from %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

    //     while (1) {
    //         char expression[MAX_EXPRESSION_SIZE];
    //         int bytesReceived = recv(clientSocket, expression, sizeof(expression), 0);

    //         if (bytesReceived <= 0) {
    //             // Error or client closed connection
    //             printf("Client closed connection\n");
    //             break;
    //         }

    //         expression[bytesReceived] = '\0';

    //         if (strcmp(expression, "-1") == 0) {
    //             // Client wants to quit
    //             printf("Client closed connection\n");
    //             break;
    //         }

    //         // Evaluate the expression
    //         double result = evaluateExpression(expression);

    //         char resultStr[50];
    //         snprintf(resultStr, sizeof(resultStr), "%.2f", result);

    //         // Send result back to client
    //         send(clientSocket, resultStr, strlen(resultStr), 0);
    //     }

    //     close(clientSocket);
    // }
while (1) {
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (clientSocket == -1) {
        perror("Accepting connection failed");
        continue;
    }

    // Print client information
    printf("TCP client connected from %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

    while (1) {
        char expression[MAX_EXPRESSION_SIZE];
        int bytesReceived = recv(clientSocket, expression, sizeof(expression), 0);

        if (bytesReceived <= 0) {
            // Error or client closed connection
            printf("Client closed connection\n");
            break;
        }

        expression[bytesReceived] = '\0';

        if (strcmp(expression, "-1") == 0) {
            // Client wants to quit
            printf("Client closed connection\n");
            break;
        }

        // Print the received expression
        printf("Received from client: %s\n", expression);

        // Evaluate the expression
        double result = evaluateExpression(expression);

        char resultStr[50];
        snprintf(resultStr, sizeof(resultStr), "%.2f", result);

        // Send result back to client
        send(clientSocket, resultStr, strlen(resultStr), 0);

        // print the answer that is being sent 
        printf("Sendind to client: %s\n", resultStr);
    }

    close(clientSocket);
}


    close(serverSocket);
    return 0;
}