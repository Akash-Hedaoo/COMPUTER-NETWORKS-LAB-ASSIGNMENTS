#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void print_window(int base, int window_size, int no_of_packets)
{
    printf("Window: [ ");
    for (int i = base; i < base + window_size && i <= no_of_packets; i++)
        printf("%d ", i);
    printf("]\n");
}

int main()
{
    int window_size, no_of_packets, nth_lost;
    int base = 1;         // oldest packet not yet acked
    int next_to_send = 1; // next packet to send
    int packets_sent;     // packets sent in this round (including slides)
    int check_packet;     // packet whose reply is being read
    int ack_no;

    // 1. Input
    printf("Enter window size: ");
    scanf("%d", &window_size);
    printf("Enter no of packets: ");
    scanf("%d", &no_of_packets);
    printf("Every nth packet lost (0 = none): ");
    scanf("%d", &nth_lost);

    // 2. Connect and share parameters
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8080);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("connect");
        return 1;
    }

    send(client_socket, &window_size, sizeof(window_size), 0);
    send(client_socket, &no_of_packets, sizeof(no_of_packets), 0);
    send(client_socket, &nth_lost, sizeof(nth_lost), 0);

    // 3. Rounds, until every packet is acked
    while (base <= no_of_packets)
    {
        printf("\n");
        print_window(base, window_size, no_of_packets);

        // Send initial window of packets
        packets_sent = 0;
        while (next_to_send < base + window_size && next_to_send <= no_of_packets)
        {
            printf("Sending packet %d\n", next_to_send);
            send(client_socket, &next_to_send, sizeof(int), 0);
            next_to_send++;
            packets_sent++;
        }

        // Process replies; if an ACK slides the window, send the newly entered packet
        check_packet = base;
        for (int i = 0; i < packets_sent; i++)
        {
            recv(client_socket, &ack_no, sizeof(ack_no), MSG_WAITALL);

            if (ack_no != -1)
            {
                // 1. Increase base first
                base = ack_no + 1;
                printf("ACK %d received, new base = %d\n", ack_no, base);

                // 2. Print the new shifted window (if packets remain)
                if (base <= no_of_packets)
                {
                    print_window(base, window_size, no_of_packets);
                }

                // 3. Send the next packet entering the new window (if available)
                if (next_to_send < base + window_size && next_to_send <= no_of_packets)
                {
                    printf("Sending packet %d\n", next_to_send);
                    send(client_socket, &next_to_send, sizeof(int), 0);
                    next_to_send++;
                    packets_sent++; // Also wait for the ACK of this newly sent packet
                }
            }
            else
            {
                printf("Packet %d not acked (lost or out of order)\n", check_packet);
            }

            check_packet++;
        }

        // 4. Go Back N: if not all packets are acked, reset next_to_send to base
        if (base <= no_of_packets)
        {
            printf("Go back to packet (new base): %d\n", base);
            next_to_send = base;
        }
    }

    printf("\nAll packets sent and acknowledged!\n");
    close(client_socket);
    return 0;
}