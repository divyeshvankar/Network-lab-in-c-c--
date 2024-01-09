
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <openssl/evp.h>  // Include OpenSSL for base64 encoding and decoding

#define SIZE 1000
int main() {
    int socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(9002);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    connect(socketDescriptor, (struct sockaddr *)&serverAddress, sizeof(serverAddress));

    char serverResponse[SIZE];
    int receivedLength = recv(socketDescriptor, serverResponse, sizeof(serverResponse), 0);

    if (receivedLength > 0) {
        // Decode the base64-encoded message received from the server
        unsigned char decodedData[SIZE];
        int decodedLength = EVP_DecodeBlock(decodedData, (unsigned char *)serverResponse, receivedLength);

        if (decodedLength > 0) {
            printf("The server sent the data: %.*s\n", decodedLength, decodedData);
        } else {
            printf("Failed to decode the received data.\n");
        }
    } else {
        printf("Failed to receive data from the server.\n");
    }

    close(socketDescriptor);
    return 0;
}
