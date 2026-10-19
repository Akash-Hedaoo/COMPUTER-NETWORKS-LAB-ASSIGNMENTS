#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_PACKETS 100

int main()
{
    int sock;
    struct sockaddr_in server_addr;

    float tp, tt;
    int window_size;
    int total_packets;

    int lost_count;
    int lost[MAX_PACKETS];

    int total_transmissions = 0;
    int total_retransmissions = 0;

    int acked[MAX_PACKETS] = {0};

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

    printf("Enter Selective Repeat window size: ");
    scanf("%d", &window_size);

    printf("Enter number of packets: ");
    scanf("%d", &total_packets);

    printf("Enter number of lost packets: ");
    scanf("%d", &lost_count);

    printf("Enter lost packet numbers:\n");

    for (int i = 0; i < lost_count; i++)
    {
        scanf("%d", &lost[i]);
    }

    /* ---------------- SEND INPUTS ---------------- */

    send(sock,
         &window_size,
         sizeof(window_size), 0);

    send(sock,
         &total_packets,
         sizeof(total_packets), 0);

    send(sock,
         &lost_count,
         sizeof(lost_count), 0);

    send(sock,
         lost,
         sizeof(int) * lost_count, 0);

    /* ---------------- HEADER ---------------- */

    float rtt = 2 * (tp + tt);

    float current_time = 1.0;

    printf("\n============================================\n");
    printf("        SELECTIVE REPEAT ARQ\n");
    printf("============================================\n");

    printf("tp                  : %.2f ms\n", tp);
    printf("tt                  : %.2f ms\n", tt);
    printf("RTT                 : %.2f ms\n", rtt);
    printf("Window size         : %d\n", window_size);
    printf("Total packets       : %d\n", total_packets);

    printf("Lost packets        : ");

    for (int i = 0; i < lost_count; i++)
    {
        printf("%d ", lost[i]);
    }

    printf("\n");

    /*
        ------------------------------------------------
        TABLE HEADER
        ------------------------------------------------
    */

    printf("\n");
    printf("============================================================\n");
    printf("  TIME      T (Transmitted)    A (ACK)    O (Remaining)\n");
    printf("============================================================\n");

    /*
        Initial window
    */

    int base = 1;

    while (base <= total_packets)
    {
        int window_end =
            base + window_size - 1;

        if (window_end > total_packets)
            window_end = total_packets;

        /*
            Send packets in current window.
        */

        for (int i = base;
             i <= window_end;
             i++)
        {
            if (acked[i] == 1)
                continue;

            int packet = i;

            int is_lost = 0;

            /*
                Check whether packet is lost.
            */

            for (int j = 0;
                 j < lost_count;
                 j++)
            {
                if (lost[j] == i)
                {
                    is_lost = 1;
                    break;
                }
            }

            /*
                ---------------------------
                TRANSMISSION
                ---------------------------
            */

            printf("  %.2f      ",
                   current_time);

            printf("{");

            for (int j = base;
                 j <= window_end;
                 j++)
            {
                if (acked[j] == 0)
                    printf("%d ", j);
            }

            printf("}");

            printf("            ");

            send(sock,
                 &packet,
                 sizeof(packet), 0);

            int type = 1;

            send(sock,
                 &type,
                 sizeof(type), 0);

            total_transmissions++;

            printf("-          ");

            int remaining =
                total_packets - i;

            printf("%d\n", remaining);

            /*
                If packet is lost, do not receive ACK.
            */

            if (is_lost)
            {
                printf("             Packet %d LOST\n",
                       i);

                /*
                    Wait for timeout.

                    For this implementation:
                    timeout = 3 RTT
                */

                current_time += 3 * rtt;

                printf("             Timeout for Packet %d\n",
                       i);

                /*
                    Retransmit ONLY this packet.
                */

                printf("             Retransmitting Packet %d\n",
                       i);

                send(sock,
                     &packet,
                     sizeof(packet), 0);

                type = 2;

                send(sock,
                     &type,
                     sizeof(type), 0);

                total_transmissions++;
                total_retransmissions++;

                current_time += rtt;

                acked[i] = 1;

                printf("             ACK %d received\n",
                       i);
            }
            else
            {
                /*
                    Normal ACK
                */

                int ack;

                recv(sock,
                     &ack,
                     sizeof(ack), 0);

                acked[ack] = 1;

                current_time += rtt;

                printf("             ACK %d received\n",
                       ack);
            }
        }

        /*
            ------------------------------------------------
            SLIDE WINDOW
            ------------------------------------------------
        */

        while (base <= total_packets &&
               acked[base] == 1)
        {
            base++;
        }
    }

    /*
        Completion signal
    */

    int end_signal = -1;

    send(sock,
         &end_signal,
         sizeof(end_signal), 0);

    /*
        ------------------------------------------------
        FINAL RESULT
        ------------------------------------------------
    */

    printf("\n");
    printf("============================================================\n");
    printf("                         RESULT\n");
    printf("============================================================\n");

    printf("Window size                  : %d\n",
           window_size);

    printf("Total packets                : %d\n",
           total_packets);

    printf("Lost packets                 : %d\n",
           lost_count);

    printf("Total transmissions          : %d\n",
           total_transmissions);

    printf("Total retransmissions        : %d\n",
           total_retransmissions);

    printf("RTT                          : %.2f ms\n",
           rtt);

    printf("Start time                   : 1.00 ms\n");

    printf("Final time                   : %.2f ms\n",
           current_time);

    printf("Total transmission time      : %.2f ms\n",
           current_time - 1);

    printf("============================================================\n");

    close(sock);

    return 0;
}
