#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <math.h>

#define PORT 8080

int main() {
    int sock;
    struct sockaddr_in server_addr;

    float tp, tt, a, max_packets;
    int n, power, window_size, total_packets, loss_n;
    int total_transmissions = 0, total_retransmissions = 0;
    int lost_packet;
    int ack_received; 

    /* Create socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket error");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* Connect */
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection error");
        exit(1);
    }

    /* INPUTS */
    printf("Enter propagation time (tp): ");
    if (scanf("%f", &tp) != 1) return 1;

    printf("Enter transmission time (tt): ");
    if (scanf("%f", &tt) != 1) return 1;

    if (tt <= 0) {
        printf("Transmission time must be greater than 0.\n");
        close(sock);
        return 0;
    }

    printf("Enter total number of packets: ");
    if (scanf("%d", &total_packets) != 1) return 1;

    printf("Enter every nth packet to be lost: ");
    if (scanf("%d", &loss_n) != 1) return 1;

    /* CALCULATE WINDOW SIZE */
    a = tp / tt;
    max_packets = 1 + 2 * a;
    n = (int)floor(log2(max_packets));
    power = 1 << n;
    window_size = (int)floor(fmin(max_packets, power));

    printf("\n--> Calculated Window Size: %d\n", window_size);

    lost_packet = loss_n;

    /* Send setup data to server */
    send(sock, &window_size, sizeof(window_size), 0);
    send(sock, &total_packets, sizeof(total_packets), 0);
    send(sock, &loss_n, sizeof(loss_n), 0);

    printf("\n====================================\n");
    printf("            GBN ARQ\n");
    printf("====================================\n");
    printf("Window size       : %d\n", window_size);
    printf("Total packets     : %d\n", total_packets);
    printf("Every nth lost    : %d\n", loss_n);

    /* INITIAL TRANSMISSION */
    int last_sent = loss_n + window_size - 1;
    if (last_sent > total_packets) last_sent = total_packets;

    printf("\nInitial transmission:\n");
    for (int i = 1; i <= last_sent; i++) {
        printf("%d ", i);
        send(sock, &i, sizeof(i), 0);
        int type = 1; // new transmission
        send(sock, &type, sizeof(type), 0);
        total_transmissions++;
        
        /* Read ACK from server to clear the network buffer */
        recv(sock, &ack_received, sizeof(ack_received), 0);
    }
    printf("\n");

    /* GBN RETRANSMISSION CYCLES */
    while (lost_packet <= total_packets) {
        if (lost_packet > last_sent) break;

        printf("\nPacket %d is LOST (Timeout occurs)\n", lost_packet);
        printf("Retransmitting from %d: ", lost_packet);

        for (int i = lost_packet; i <= last_sent; i++) {
            printf("%d ", i);
            send(sock, &i, sizeof(i), 0);
            int type = 2; // retransmission
            send(sock, &type, sizeof(type), 0);
            total_transmissions++;
            total_retransmissions++;
            
            recv(sock, &ack_received, sizeof(ack_received), 0);
        }
        printf("\n");

        int next_loss = lost_packet + window_size - 1;
        int new_last = next_loss + window_size - 1;
        if (new_last > total_packets) new_last = total_packets;

        if (last_sent < new_last) {
            printf("New packets sent: ");
            for (int i = last_sent + 1; i <= new_last; i++) {
                printf("%d ", i);
                send(sock, &i, sizeof(i), 0);
                int type = 1;
                send(sock, &type, sizeof(type), 0);
                total_transmissions++;
                
                recv(sock, &ack_received, sizeof(ack_received), 0);
            }
            printf("\n");
        }

        last_sent = new_last;
        lost_packet = next_loss;
    }

    /* FINAL RETRANSMISSION */
    if (lost_packet <= total_packets) {
        printf("\nPacket %d is LOST\n", lost_packet);
        printf("Final retransmission: ");
        for (int i = lost_packet; i <= total_packets; i++) {
            printf("%d ", i);
            send(sock, &i, sizeof(i), 0);
            int type = 2;
            send(sock, &type, sizeof(type), 0);
            total_transmissions++;
            total_retransmissions++;
            
            recv(sock, &ack_received, sizeof(ack_received), 0);
        }
        printf("\n");
    }

    /* Send completion signal */
    int end_signal = -1;
    send(sock, &end_signal, sizeof(end_signal), 0);

    /* FINAL RESULT */
    printf("\n====================================\n");
    printf("              RESULT\n");
    printf("====================================\n");
    printf("Window size                  : %d\n", window_size);
    printf("Total packets                : %d\n", total_packets);
    printf("Every nth packet lost        : %d\n", loss_n);
    printf("Total transmissions required : %d\n", total_transmissions);
    printf("Total retransmissions        : %d\n", total_retransmissions);
    printf("====================================\n");

    close(sock);
    return 0;
}