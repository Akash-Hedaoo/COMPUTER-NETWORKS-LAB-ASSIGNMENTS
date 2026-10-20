#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define KEY "1011011000"

// Custom XOR function for binary characters
char customXor(char a, char b) {
    return (a == b) ? '0' : '1';
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char data[1000] = {0};

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Attach socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the network address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    // Read the spread data from the client
    read(new_socket, data, 1000);
    printf("\nReceived Spread Data: %s\n", data);

    int dataLen = strlen(data);
    int keyLen = strlen(KEY);

    if (dataLen % keyLen != 0 || dataLen == 0) {
        printf("\nError: Corrupted data (length mismatch).\n");
    } else {
        printf("Original Data: ");
        for (int i = 0; i < dataLen; i += keyLen) {
            char origBit = customXor(data[i], KEY[0]);
            printf("%c", origBit);
        }
        printf("\n");
    }

    close(new_socket);
    close(server_fd);
    return 0;
}