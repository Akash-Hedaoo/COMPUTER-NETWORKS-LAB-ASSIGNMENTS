#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

typedef struct {
    int data;
    int prop_time;
} packet;

typedef struct {
    int ack_no;
    int rtt;
} ack;

// Function check karne ke liye ki packet lost array mein hai ya nahi
int is_lost(int pkt, int* lost_nos, int num_lost) {
    for (int i = 0; i < num_lost; i++) {
        if (lost_nos[i] == pkt) return 1;
    }
    return 0;
}

int main() {
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Error creating the socket ");
        return 1;
    }

    struct sockaddr_in client_address;
    client_address.sin_family = AF_INET;
    client_address.sin_port = htons(8080);
    client_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    if (connect(client_socket, (struct sockaddr *)&client_address, sizeof(client_address)) < 0) {
        perror("Error connecting the socket ");
        return 1;
    }

    int tt, tp, no_of_packets, num_lost;
    printf("Enter Tt : ");
    scanf("%d", &tt);
    printf("Enter Tp : ");
    scanf("%d", &tp);
    printf("Enter no of packets : ");
    scanf("%d", &no_of_packets);
    printf("Enter no of packets to lost : ");
    scanf("%d", &num_lost);

    int lost_nos[100] = {0};
    if (num_lost > 0) {
        printf("Enter the lost nos (space separated) : ");
        for (int i = 0; i < num_lost; i++) {
            scanf("%d", &lost_nos[i]);
        }
    }

    // sending data to server
    send(client_socket, &tt, sizeof(tt), 0);
    send(client_socket, &tp, sizeof(tp), 0);
    send(client_socket, &no_of_packets, sizeof(no_of_packets), 0);
    send(client_socket, &num_lost, sizeof(num_lost), 0);
    if (num_lost > 0) {
        send(client_socket, lost_nos, sizeof(int) * num_lost, 0);
    }

    int rtt_calculated = 2 * (tt + tp);
    int to_calculated = 3 * rtt_calculated;
    
    printf("Transmission time = %d \n", tt);
    printf("Propagation time = %d \n", tp);
    printf("Round trip time 2(tt + tp) = %d \n", rtt_calculated);
    printf("Timeout time 3(Rtt) = %d \n", to_calculated);

    int curr_time = 0;
    int curr_packet = 1;

    while (curr_packet <= no_of_packets) {
        curr_time += tt;
        printf("--------------------------------------------------------------------\n");
        printf("Packet %d sent at t = %d\n", curr_packet, curr_time);

        packet p = {curr_packet, curr_time + tp};
        send(client_socket, &p, sizeof(p), 0);

        if (is_lost(curr_packet, lost_nos, num_lost)) {
            // Case 2: Packet is lost
            // Client ko pata hai timeout hoga, isliye recv() pe nahi rukega
            curr_time += to_calculated;
            printf("Timeout occurred! Retransmitting packet...\n");
            
            // Retransmit
            curr_time += tt;
            printf("Packet %d re-sent at t = %d\n", curr_packet, curr_time);
            p.prop_time = curr_time + tp;
            send(client_socket, &p, sizeof(p), 0);

            // Ab ACK receive karo (kyunki server dusri baar mein pakka bhejega)
            ack a;
            recv(client_socket, &a, sizeof(a), 0);
            printf("Packet ack recieved at t = %d\n", curr_time + a.rtt);
            curr_time += a.rtt;

        } else {
            // Case 1: Packet not lost
            ack a;
            recv(client_socket, &a, sizeof(a), 0);
            printf("Packet ack recieved at t = %d\n", curr_time + a.rtt);
            curr_time += a.rtt;
        }

        curr_packet++;
    }

    printf("--------------------------------------------------------------------\n");
    printf("Transmission Completed \n");
    
    int n1 = no_of_packets - num_lost;
    int n2 = num_lost;
    int formula_time = n1 * (2 * (tt + tp)) + n2 * (4 * rtt_calculated + 2 * tt);
    
    printf("Total simulated time = %d \n", curr_time);
    printf("Verified formula time n1(2(tt + tp)) + n2(4rtt + 2tt) = %d \n", formula_time);

    close(client_socket);
    return 0;
}