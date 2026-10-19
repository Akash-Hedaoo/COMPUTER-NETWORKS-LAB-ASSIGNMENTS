#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_PACKETS 100

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len;

    int window_size;
    int total_packets;
    int lost_count;
    int lost[MAX_PACKETS];

    int received[MAX_PACKETS] = {0};

    /* ---------------- SOCKET ---------------- */

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("Socket error");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("Bind error");
        exit(1);
    }

    listen(server_fd, 5);

    printf("Waiting for sender...\n");

    addr_len = sizeof(client_addr);

    client_fd = accept(server_fd,
                       (struct sockaddr *)&client_addr,
                       &addr_len);

    if (client_fd < 0)
    {
        perror("Accept error");
        exit(1);
    }

    printf("Sender connected.\n\n");

    /* ---------------- RECEIVE INPUTS ---------------- */

    recv(client_fd,
         &window_size,
         sizeof(window_size), 0);

    recv(client_fd,
         &total_packets,
         sizeof(total_packets), 0);

    recv(client_fd,
         &lost_count,
         sizeof(lost_count), 0);

    recv(client_fd,
         lost,
         sizeof(int) * lost_count, 0);

    printf("============================================\n");
    printf("        SELECTIVE REPEAT ARQ - SERVER\n");
    printf("============================================\n");

    printf("Window size       : %d\n", window_size);
    printf("Total packets     : %d\n", total_packets);
    printf("Number lost       : %d\n", lost_count);

    printf("Lost packets      : ");

    for (int i = 0; i < lost_count; i++)
    {
        printf("%d ", lost[i]);
    }

    printf("\n\n");

    /* Mark packets whose FIRST transmission is lost */

    int first_attempt[MAX_PACKETS] = {0};

    for (int i = 0; i < lost_count; i++)
    {
        first_attempt[lost[i]] = 1;
    }

    /* ---------------- RECEIVE PACKETS ---------------- */

    while (1)
    {
        int packet;
        int type;

        int bytes = recv(client_fd,
                         &packet,
                         sizeof(packet), 0);

        if (bytes <= 0)
            break;

        /*
            -1 is completion signal
        */

        if (packet == -1)
            break;

        recv(client_fd,
             &type,
             sizeof(type), 0);

        printf("--------------------------------------------\n");

        if (type == 1)
        {
            printf("Received Packet %d\n", packet);
        }
        else
        {
            printf("Received Retransmitted Packet %d\n",
                   packet);
        }

        /*
            First transmission of a lost packet
        */

        if (first_attempt[packet] == 1)
        {
            printf("Packet %d LOST\n", packet);

            first_attempt[packet] = 0;

            /*
                No ACK is sent.
            */

            printf("ACK %d NOT sent\n", packet);

            continue;
        }

        /*
            Packet successfully received.
        */

        if (received[packet] == 0)
        {
            received[packet] = 1;

            printf("Packet %d accepted\n", packet);

            /*
                Selective Repeat:
                ACK this packet individually.
            */

            int ack = packet;

            send(client_fd,
                 &ack,
                 sizeof(ack), 0);

            printf("ACK %d sent\n", ack);
        }
        else
        {
            /*
                Duplicate packet
            */

            int ack = packet;

            printf("Duplicate Packet %d\n",
                   packet);

            send(client_fd,
                 &ack,
                 sizeof(ack), 0);

            printf("ACK %d sent again\n", ack);
        }
    }

    printf("\n============================================\n");
    printf("All packets processed.\n");
    printf("============================================\n");

    close(client_fd);
    close(server_fd);

    return 0;
}

