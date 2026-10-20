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
    int sock = 0;
    struct sockaddr_in serv_addr;
    char data[100];
    char ans[1000];
    int ptr = 0;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    printf("Enter Binary Data: ");
    scanf("%s", data);

    int dataLen = strlen(data);
    int keyLen = strlen(KEY);

    // Spread the data
    for (int i = 0; i < dataLen; i++) {
        for (int j = 0; j < keyLen; j++) {
            ans[ptr++] = customXor(data[i], KEY[j]);
        }
    }
    ans[ptr] = '\0';

    printf("\nTransmitted (Spread) Data: %s\n", ans);
    
    // Send spread data to server
    send(sock, ans, strlen(ans), 0);
    printf("Data sent to server successfully.\n");

    close(sock);
    return 0;
}