#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_BITS 100

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len;

    int codeword[MAX_BITS];
    int data[MAX_BITS];
    int m, r;

    /* ---------------- SOCKET SETUP ---------------- */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket error");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind error");
        exit(1);
    }

    listen(server_fd, 5);
    printf("Waiting for sender...\n");

    addr_len = sizeof(client_addr);
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_fd < 0) {
        perror("Accept error");
        exit(1);
    }
    printf("Sender connected.\n\n");

    /* ---------------- RECEIVE CODEWORD ---------------- */
    recv(client_fd, &m, sizeof(m), 0);
    recv(client_fd, &r, sizeof(r), 0);
    recv(client_fd, codeword, sizeof(int) * (m + r + 1), 0);

    printf("====================================\n");
    printf("       HAMMING CODE RECEIVER        \n");
    printf("====================================\n");

    printf("Received Codeword: ");
    for (int i = 1; i <= m + r; i++) {
        printf("%d ", codeword[i]);
    }
    printf("\n");

    /* ---------------- ERROR DETECTION ---------------- */
    int error_pos = 0;

    for (int i = 0; i < r; i++) {
        int pos = (1 << i);
        int parity = 0;
        for (int x = 1; x <= m + r; x++) {
            if ((x & pos) != 0) {
                parity ^= codeword[x];
            }
        }
        if (parity != 0) {
            error_pos += pos; // Accumulate syndrome bits
        }
    }

    /* ---------------- ERROR CORRECTION ---------------- */
    if (error_pos == 0) {
        printf("\nResult: No error detected in transmission.\n");
    } else {
        printf("\nResult: Error detected at position %d!\n", error_pos);
        
        if (error_pos <= m + r) {
            codeword[error_pos] ^= 1; // Flip the erroneous bit to correct it
            printf("Correcting error...\n");
            printf("Corrected Codeword: ");
            for (int i = 1; i <= m + r; i++) {
                printf("%d ", codeword[i]);
            }
            printf("\n");
        } else {
            printf("Error position out of bounds (multiple bits may be corrupted).\n");
        }
    }

    /* ---------------- EXTRACT ORIGINAL DATA ---------------- */
    int j = 0;
    int k = 0;
    for (int i = 1; i <= m + r; i++) {
        if (i == (1 << k)) {
            k++;
        } else {
            data[j++] = codeword[i];
        }
    }

    printf("\nExtracted Data Bits: ");
    for (int i = 0; i < m; i++) {
        printf("%d", data[i]);
    }
    printf("\n====================================\n");

    close(client_fd);
    close(server_fd);
    return 0;
}