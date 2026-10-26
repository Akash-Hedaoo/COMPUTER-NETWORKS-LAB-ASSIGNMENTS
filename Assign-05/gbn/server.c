#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main()
{
    int window_size, no_of_packets, nth_lost;
    int packet_no, ack_no;
    int expected_packet = 1;
    int count = 0; // Counts 1, 2, ..., nth_lost for expected packets

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8080);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("bind");
        close(server_socket);
        return 1;
    }

    listen(server_socket, 3);
    printf("Waiting for client...\n");

    int client_socket = accept(server_socket, NULL, NULL);
    if (client_socket < 0)
    {
        perror("accept");
        close(server_socket);
        return 1;
    }

    // Receive parameters
    recv(client_socket, &window_size, sizeof(window_size), MSG_WAITALL);
    recv(client_socket, &no_of_packets, sizeof(no_of_packets), MSG_WAITALL);
    recv(client_socket, &nth_lost, sizeof(nth_lost), MSG_WAITALL);

    printf("Window = %d, Packets = %d, Lost every = %d\n\n",
           window_size, no_of_packets, nth_lost);

    while (expected_packet <= no_of_packets)
    {
        if (recv(client_socket, &packet_no, sizeof(packet_no), MSG_WAITALL) != sizeof(packet_no))
            break;

        ack_no = -1;

        if (packet_no == expected_packet)
        {
            count++; // 1st, 2nd, 3rd in-order arrival

            if (nth_lost != 0 && count == nth_lost)
            {
                printf("Packet %d simulated as lost\n", packet_no);
                count = 0; // Reset so the next 3rd in-order arrival (3 -> 4 -> 5) is lost!
            }
            else
            {
                printf("Packet %d accepted, sending ACK\n", packet_no);
                ack_no = packet_no;
                expected_packet++;
            }
        }
        else
        {
            printf("Packet %d discarded (out of order, expected %d)\n", packet_no, expected_packet);
        }

        if (send(client_socket, &ack_no, sizeof(ack_no), 0) <= 0)
            break;
    }

    close(client_socket);
    close(server_socket);
    return 0;
}