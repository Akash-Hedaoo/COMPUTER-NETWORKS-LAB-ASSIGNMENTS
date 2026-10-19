#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <math.h>

#define PORT 8080
#define MAX_PACKETS 10000

int main()
{
    int sock;
    struct sockaddr_in server_addr;

    float tp, tt;
    float a;
    float max_packets;

    int n;
    int power;
    int window_size;
    int total_packets;

    int total_transmissions;
    int total_acks;

    float end_time;

    /* ---------------- SOCKET ---------------- */

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        perror("Socket error");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr =
        inet_addr("127.0.0.1");

    if (connect(sock,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("Connection error");
        exit(1);
    }

    /* ---------------- INPUT ---------------- */

    printf("Enter propagation time (tp): ");
    scanf("%f", &tp);

    printf("Enter transmission time (tt): ");
    scanf("%f", &tt);

    if (tt <= 0)
    {
        printf("Transmission time must be greater than 0.\n");
        close(sock);
        return 0;
    }

    /* ---------------- CALCULATIONS ---------------- */

    a = tp / tt;

    max_packets = 1 + 2 * a;

    n = (int)floor(log2(max_packets));

    power = 1 << n;

    window_size = (int)floor(
        fmin(max_packets, power)
    );

    /*
       Maximum number of packets that can be
       transmitted in the calculation.
    */
    total_packets = (int)floor(max_packets);

    total_transmissions = total_packets;

    /*
       One ACK for every successfully received packet.
    */
    total_acks = total_packets;

    /*
       ------------------------------------------------
       SLIDING WINDOW TIMING
       ------------------------------------------------

       Start time = 1 ms

       Sender transmits one packet every tt ms.

       ACK travels back after propagation delay.

       We maintain:
           send_time[i]
           ack_time[i]
    */

    float send_time[MAX_PACKETS];
    float ack_time[MAX_PACKETS];

    int base = 1;
    int next_packet = 1;

    float sender_free = 1.0;

    /*
       ACK return delay.

       The round-trip propagation delay is:

              2 * tp
    */
    float ack_delay = 2 * tp;

    while (base <= total_packets)
    {
        /*
           Fill the current window.
        */

        while (next_packet <= total_packets &&
               next_packet < base + window_size)
        {
            send_time[next_packet] = sender_free;

            /*
               ACK is scheduled after the
               round-trip propagation time.
            */
            ack_time[next_packet] =
                sender_free + ack_delay;

            printf("Packet %d sent at %.2f ms\n",
                   next_packet,
                   sender_free);

            sender_free += tt;

            next_packet++;
        }

        /*
           Find when the oldest unacknowledged
           packet gets its ACK.
        */

        if (ack_time[base] > sender_free)
        {
            /*
               Sender has to wait for the ACK.
            */
            sender_free = ack_time[base];
        }

        /*
           ACK received -> window slides.
        */

        printf("ACK %d received at %.2f ms\n",
               base,
               ack_time[base]);

        base++;
    }

    /*
       The process finishes when the ACK for the
       final packet arrives.
    */

    end_time = ack_time[total_packets];

    /*
       ------------------------------------------------
       SEND RESULTS TO SERVER
       ------------------------------------------------
    */

    send(sock, &tp, sizeof(tp), 0);
    send(sock, &tt, sizeof(tt), 0);
    send(sock, &a, sizeof(a), 0);
    send(sock, &max_packets,
         sizeof(max_packets), 0);

    send(sock, &n, sizeof(n), 0);
    send(sock, &power, sizeof(power), 0);
    send(sock, &window_size,
         sizeof(window_size), 0);

    send(sock, &total_packets,
         sizeof(total_packets), 0);

    send(sock, &total_transmissions,
         sizeof(total_transmissions), 0);

    send(sock, &total_acks,
         sizeof(total_acks), 0);

    send(sock, &end_time,
         sizeof(end_time), 0);

    /* ---------------- OUTPUT ---------------- */

    printf("\n====================================\n");
    printf("       SLIDING WINDOW PROTOCOL\n");
    printf("====================================\n");

    printf("Propagation time (tp)       : %.2f ms\n",
           tp);

    printf("Transmission time (tt)      : %.2f ms\n",
           tt);

    printf("a = tp / tt                 : %.2f\n",
           a);

    printf("Maximum packets (1 + 2a)    : %.2f\n",
           max_packets);

    printf("Number of bits (n)          : %d\n",
           n);

    printf("2^n                         : %d\n",
           power);

    printf("Window size                 : %d\n",
           window_size);

    printf("Total packets               : %d\n",
           total_packets);

    printf("Total transmissions         : %d\n",
           total_transmissions);

    printf("Total ACKs received         : %d\n",
           total_acks);

    printf("Process ends at             : %.2f ms\n",
           end_time);

    printf("====================================\n");

    close(sock);

    return 0;
}