
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define BUFFER_SIZE 4096

typedef struct {
    int type;
    int seq;
    size_t size;
} Packet;

int main() {
    int client_socket = socket(AF_INET, SOCK_DGRAM, 0);

    if (client_socket < 0) {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);

    struct timeval timeout = {2, 0};
    setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO,
               &timeout, sizeof(timeout));

    char filepath[256];
    printf("Enter the file path: ");

    if (fgets(filepath, sizeof(filepath), stdin) == NULL) {
        close(client_socket);
        return 1;
    }

    filepath[strcspn(filepath, "\n")] = '\0';

    FILE *fp = fopen(filepath, "rb");

    if (fp == NULL) {
        perror("File open failed");
        close(client_socket);
        return 1;
    }

    char *filename = strrchr(filepath, '/');
    filename = filename ? filename + 1 : filepath;

    fseek(fp, 0, SEEK_END);
    size_t file_size = (size_t)ftell(fp);
    rewind(fp);

    socklen_t server_len = sizeof(server_address);

    // Send filename
    if (sendto(client_socket, filename, strlen(filename) + 1, 0,
               (struct sockaddr *)&server_address, server_len) < 0) {
        perror("Filename send failed");
        fclose(fp);
        close(client_socket);
        return 1;
    }

    // Send file size
    if (sendto(client_socket, &file_size, sizeof(file_size), 0,
               (struct sockaddr *)&server_address, server_len) < 0) {
        perror("File size send failed");
        fclose(fp);
        close(client_socket);
        return 1;
    }

    char buffer[BUFFER_SIZE];
    size_t total_sent = 0;
    int seq = 0;

    while (total_sent < file_size) {
        size_t bytes_read = fread(buffer, 1, sizeof(buffer), fp);

        if (bytes_read == 0) {
            break;
        }

        Packet packet = {3, seq, bytes_read};
        int acknowledged = 0;

        while (!acknowledged) {
            if (sendto(client_socket, &packet, sizeof(packet), 0,
                       (struct sockaddr *)&server_address, server_len) < 0 ||
                sendto(client_socket, buffer, bytes_read, 0,
                       (struct sockaddr *)&server_address, server_len) < 0) {
                perror("Send failed");
                fclose(fp);
                close(client_socket);
                return 1;
            }

            Packet ack;
            ssize_t n = recvfrom(client_socket, &ack, sizeof(ack), 0,
                                 NULL, NULL);

            if (n == sizeof(ack) && ack.type == 4 && ack.seq == seq) {
                acknowledged = 1;
            } else {
                printf("Timeout: resending chunk %d\n", seq);
            }
        }

        total_sent += bytes_read;
        seq++;
    }

    fclose(fp);
    close(client_socket);

    printf("File sent successfully!\n");
    printf("Total bytes sent: %zu\n", total_sent);

    return 0;
}
