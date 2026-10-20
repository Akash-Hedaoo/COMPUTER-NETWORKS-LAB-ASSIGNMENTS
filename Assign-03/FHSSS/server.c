#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

// Custom XOR function
int XOR(int a, int b) {
    if (a == b)
        return 0;
    else
        return 1;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    int hop[] = {1, 3, 2, 0};
    int n = 4;
    int count;
    int channel[100];

    // Create socket file descriptor (TCP)
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

    // Bind the socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server (Receiver) listening on port %d...\n", PORT);

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    // Read the number of channels first
    read(new_socket, &count, sizeof(int));
    
    // Read the actual channel array
    read(new_socket, channel, count * sizeof(int));

    printf("\nReceived %d FHSS channels: ", count);
    for (int i = 0; i < count; i++) {
        printf("%d ", channel[i]);
    }
    printf("\n");

    printf("Decoded Data: ");
    for (int i = 0; i < count; i++) {
        // Get encoded bit
        int encoded = (channel[i] - hop[i % n] + 4) % 4;

        // XOR
        int bit = XOR(encoded % 2, hop[i % n] % 2);

        printf("%d", bit);
    }
    printf("\n");

    close(new_socket);
    close(server_fd);
    return 0;
}