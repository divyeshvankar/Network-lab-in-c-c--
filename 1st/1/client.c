#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/evp.h>

#define MSG_LEN 1000
int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <Server_IP_Address> <Server_Port_Number>\n", argv[0]);
        return 1;
    }

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Error creating client socket");
        return 1;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &server_address.sin_addr) <= 0) {
        perror("Invalid server IP address");
        close(client_socket);
        return 1;
    }

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Error connecting to server");
        close(client_socket);
        return 1;
    }

    while (1) {
        char message[MSG_LEN];
        printf("Enter a message to send (or 'exit' to quit): ");
        fgets(message, sizeof(message), stdin);
        message[strcspn(message, "\n")] = '\0'; // Remove newline character

        // Check if the user wants to exit
        if (strcmp(message, "exit") == 0) {
            printf("Exiting the client.\n");
            break; // Exit the loop
        }

        
        // Encode the message using Base64
        unsigned char encodedData[MSG_LEN * 2];
        int encodedLength = EVP_EncodeBlock(encodedData, (unsigned char *)message, strlen(message));
        
        // Send the encoded message to the server
        send(client_socket, encodedData, encodedLength, 0);
        
        // Receive acknowledgment from the server
        char acknowledgment[15];
        recv(client_socket, acknowledgment, sizeof(acknowledgment), 0);
        printf("Received acknowledgment: %s\n", acknowledgment);
    }

    close(client_socket);
    return 0;
}
