#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_BITS 100

int main() {
    int sock;
    struct sockaddr_in server_addr;

    char data_str[MAX_BITS];
    int data[MAX_BITS];
    int codeword[MAX_BITS];
    int m, r = 0;

    /* ---------------- SOCKET SETUP ---------------- */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket error");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection error");
        exit(1);
    }

    /* ---------------- INPUT ---------------- */
    printf("Enter data bits (e.g., 1011): ");
    scanf("%s", data_str);

    m = strlen(data_str);
    for (int i = 0; i < m; i++) {
        data[i] = data_str[i] - '0';
    }

    /* ---------------- CALCULATE PARITY BITS (r) ---------------- */
    while ((1 << r) < (m + r + 1)) {
        r++;
    }

    printf("\nData bits (m)   : %d\n", m);
    printf("Parity bits (r) : %d\n", r);
    printf("Total bits      : %d\n", m + r);

    /* ---------------- CONSTRUCT CODEWORD ---------------- */
    int j = 0; // data index
    int k = 0; // parity index

    for (int i = 1; i <= m + r; i++) {
        if (i == (1 << k)) {
            codeword[i] = 0; // Placeholder for parity bits
            k++;
        } else {
            codeword[i] = data[j++];
        }
    }

    /* ---------------- CALCULATE EVEN PARITY ---------------- */
    for (int i = 0; i < r; i++) {
        int pos = (1 << i);
        int parity = 0;
        for (int x = 1; x <= m + r; x++) {
            if ((x & pos) != 0) {
                parity ^= codeword[x];
            }
        }
        codeword[pos] = parity;
    }

    printf("\nGenerated Codeword: ");
    for (int i = 1; i <= m + r; i++) {
        printf("%d ", codeword[i]);
    }
    printf("\n");

    /* ---------------- INJECT ERROR ---------------- */
    int err_pos;
    printf("\nEnter position to inject error (1 to %d) or 0 for no error: ", m + r);
    scanf("%d", &err_pos);

    if (err_pos > 0 && err_pos <= m + r) {
        codeword[err_pos] ^= 1; // Flip the bit
        printf("Error injected at position %d.\n", err_pos);
        printf("Transmitted Codeword: ");
        for (int i = 1; i <= m + r; i++) {
            printf("%d ", codeword[i]);
        }
        printf("\n");
    } else {
        printf("No error injected.\n");
    }

    /* ---------------- SEND TO SERVER ---------------- */
    send(sock, &m, sizeof(m), 0);
    send(sock, &r, sizeof(r), 0);
    send(sock, codeword, sizeof(int) * (m + r + 1), 0);

    printf("\nCodeword sent to server successfully.\n");

    close(sock);
    return 0;
}