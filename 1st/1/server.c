#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <openssl/evp.h>

#define MSG_LEN 1000


void *clientHandler(void *arg) {
    int client_socket = *((int *)arg);
    
    // Receive and process messages from the client
    while (1) {
        char message[MSG_LEN];
        memset(message, 0, sizeof(message));
        
        int bytes_received = recv(client_socket, message, sizeof(message), 0);
        printf("%d\n",bytes_received);
        if (bytes_received <= 0) {
            perror("Error receiving data from client");
            break;
        }
        
        // Decode the received message
        unsigned char decodedData[MSG_LEN];
        int decodedLength = EVP_DecodeBlock(decodedData, (unsigned char *)message, bytes_received);
        
        if (decodedLength > 0) {
            decodedData[decodedLength] = '\0';
            printf("Received and decoded message from client: %s\n", decodedData);
        }
        
        // Send an acknowledgment back to the client
        send(client_socket, "Acknowledgment", 14, 0);
    }
    
    close(client_socket);
    pthread_exit(NULL);
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <Server_Port_Number>\n", argv[0]);
        return 1;
    }
    
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error creating server socket");
        return 1;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[1]));
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Error binding server socket");
        close(server_socket);
        return 1;
    }

    if (listen(server_socket, 5) == -1) {
        perror("Error listening on server socket");
        close(server_socket);
        return 1;
    }
    
    printf("Server listening on port %s...\n", argv[1]);

    while (1) {
        struct sockaddr_in client_address;
        socklen_t client_address_size = sizeof(client_address);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_size);
        if (client_socket == -1) {
            perror("Error accepting client connection");
            continue;
        }

        pthread_t thread;
        pthread_create(&thread, NULL, clientHandler, &client_socket);
        pthread_detach(thread);
    }

    close(server_socket);
    return 0;
}
