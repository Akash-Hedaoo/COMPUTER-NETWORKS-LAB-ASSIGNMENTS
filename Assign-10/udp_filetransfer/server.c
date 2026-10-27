
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 4096

typedef struct {
    int type;           // 1 = filename, 2 = filesize, 3 = data, 4 = end
    int seq;
    size_t size;
} Packet;

int main() {
    int server_socket = socket(AF_INET, SOCK_DGRAM, 0);

    if (server_socket < 0) {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in server_address, client_address;
    socklen_t client_len = sizeof(client_address);

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8080);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&server_address,
             sizeof(server_address)) < 0) {
        perror("Bind failed");
        close(server_socket);
        return 1;
    }

    printf("Waiting for the client...\n");

    char filename[256];
    size_t file_size;

    Packet packet;

    // Receive filename
    if (recvfrom(server_socket, filename, sizeof(filename), 0,
                 (struct sockaddr *)&client_address, &client_len) <= 0) {
        perror("Filename receive failed");
        close(server_socket);
        return 1;
    }

    filename[255] = '\0';

    // Receive file size
    if (recvfrom(server_socket, &file_size, sizeof(file_size), 0,
                 (struct sockaddr *)&client_address, &client_len) <= 0) {
        perror("File size receive failed");
        close(server_socket);
        return 1;
    }

    // Create output path
    char output_path[512] = "recieved_files/";

    if (snprintf(output_path + strlen(output_path),
                 sizeof(output_path) - strlen(output_path),
                 "%s", filename) >=
        (int)(sizeof(output_path) - strlen("recieved_files/"))) {
        fprintf(stderr, "Filename too long\n");
        close(server_socket);
        return 1;
    }

    FILE *fp = fopen(output_path, "wb");

    if (fp == NULL) {
        perror("File open failed");
        close(server_socket);
        return 1;
    }

    printf("Receiving file: %s\n", filename);

    size_t total_received = 0;
    int expected_seq = 0;
    char buffer[BUFFER_SIZE];

    while (total_received < file_size) {
        ssize_t n = recvfrom(server_socket, &packet, sizeof(packet), 0,
                             (struct sockaddr *)&client_address, &client_len);

        if (n != sizeof(packet)) {
            continue;
        }

        if (packet.type != 3 || packet.seq != expected_seq ||
            packet.size > BUFFER_SIZE ||
            packet.size > file_size - total_received) {
            continue;
        }

        ssize_t data_n = recvfrom(server_socket, buffer, packet.size, 0,
                                  (struct sockaddr *)&client_address, &client_len);

        if (data_n != (ssize_t)packet.size) {
            continue;
        }

        if (fwrite(buffer, 1, packet.size, fp) != packet.size) {
            perror("File write failed");
            fclose(fp);
            close(server_socket);
            return 1;
        }

        total_received += packet.size;
        expected_seq++;

        // ACK the received chunk
        Packet ack = {4, expected_seq - 1, 0};
        sendto(server_socket, &ack, sizeof(ack), 0,
               (struct sockaddr *)&client_address, client_len);
    }

    fclose(fp);

    printf("File received successfully!\n");
    printf("Total bytes received: %zu\n", total_received);

    close(server_socket);
    return 0;
}
