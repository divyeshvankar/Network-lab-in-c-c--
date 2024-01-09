#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <openssl/evp.h>  // Include OpenSSL for base64 encoding

int main() {
    char serverMessage[] = "You have a missed call from server\n";

    int socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(9002);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    bind(socketDescriptor, (struct sockaddr *)&serverAddress, sizeof(serverAddress));

    listen(socketDescriptor, 5);

    int client_socket = accept(socketDescriptor, NULL, NULL);

    // Encode the message to base64 before sending
    unsigned char encodedData[256];
    int encodedLength;
    EVP_EncodeBlock(encodedData, (unsigned char *)serverMessage, strlen(serverMessage));
    encodedLength = (strlen(serverMessage) + 2) / 3 * 4;

    // Send the base64-encoded message
    send(client_socket, encodedData, encodedLength, 0);

    close(socketDescriptor);
    return 0;
}
// 91.189.91.38 80