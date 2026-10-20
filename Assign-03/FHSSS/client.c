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
    int sock = 0;
    struct sockaddr_in serv_addr;
    
    char data[100];
    int channels[100];
    int hop[] = {1, 3, 2, 0};
    int n = 4;

    // Create socket (TCP)
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    printf("Enter binary data: ");
    scanf("%s", data);

    int count = strlen(data);

    printf("FHSS Channels Generated: ");
    for (int i = 0; i < count; i++) {
        int bit = data[i] - '0';

        // XOR
        int encoded = XOR(bit, hop[i % n] % 2);

        // Calculate channel
        channels[i] = (hop[i % n] + encoded) % 4;

        printf("%d ", channels[i]);
    }
    printf("\n");

    // Send the number of channels to the server
    send(sock, &count, sizeof(int), 0);
    
    // Send the array of generated channels
    send(sock, channels, count * sizeof(int), 0);
    
    printf("\nData sent to server successfully.\n");

    close(sock);
    return 0;
}