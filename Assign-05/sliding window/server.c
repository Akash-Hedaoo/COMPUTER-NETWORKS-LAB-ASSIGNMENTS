#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len;

    float tp, tt, a, max_packets, end_time;
    int n, power, window_size, total_packets, total_transmissions, total_acks;

    /* ---------------- SOCKET ---------------- */
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
    printf("Waiting for client...\n");

    addr_len = sizeof(client_addr);
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_fd < 0) {
        perror("Accept error");
        exit(1);
    }
    printf("Client connected.\n\n");

    /* ---------------- RECEIVE ---------------- */
    recv(client_fd, &tp, sizeof(tp), 0);
    recv(client_fd, &tt, sizeof(tt), 0);
    recv(client_fd, &a, sizeof(a), 0);
    recv(client_fd, &max_packets, sizeof(max_packets), 0);
    recv(client_fd, &n, sizeof(n), 0);
    recv(client_fd, &power, sizeof(power), 0);
    recv(client_fd, &window_size, sizeof(window_size), 0);
    recv(client_fd, &total_packets, sizeof(total_packets), 0);
    recv(client_fd, &total_transmissions, sizeof(total_transmissions), 0);
    recv(client_fd, &total_acks, sizeof(total_acks), 0);
    recv(client_fd, &end_time, sizeof(end_time), 0);

    /* ---------------- OUTPUT ---------------- */
    printf("====================================\n");
    printf("       SLIDING WINDOW PROTOCOL      \n");
    printf("====================================\n");
    printf("Propagation time (tp)       : %.2f ms\n", tp);
    printf("Transmission time (tt)      : %.2f ms\n", tt);
    printf("a = tp / tt                 : %.2f\n", a);
    printf("Maximum packets (1 + 2a)    : %.2f\n", max_packets);
    printf("Number of bits (n)          : %d\n", n);
    printf("2^n                         : %d\n", power);
    printf("Window size                 : %d\n", window_size);
    printf("Total packets               : %d\n", total_packets);
    printf("Total transmissions         : %d\n", total_transmissions);
    printf("Total ACKs received         : %d\n", total_acks);
    printf("Process ends at             : %.2f ms\n", end_time);
    printf("====================================\n");

    close(client_fd);
    close(server_fd);
    return 0;
}