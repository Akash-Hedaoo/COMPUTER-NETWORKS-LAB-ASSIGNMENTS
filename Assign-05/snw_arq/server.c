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
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error creating the socket");
        return 1;
    }

    struct sockaddr_in server_address, client_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8080);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Error binding");
        return 1;
    }

    listen(server_socket, 5);
    printf("Server listening on port 8080...\n");

    socklen_t client_len = sizeof(client_address);
    int client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_len);

    int tt, tp, no_of_packets, num_lost;
    recv(client_socket, &tt, sizeof(tt), 0);
    recv(client_socket, &tp, sizeof(tp), 0);
    recv(client_socket, &no_of_packets, sizeof(no_of_packets), 0);
    recv(client_socket, &num_lost, sizeof(num_lost), 0);

    int lost_nos[100] = {0};
    if (num_lost > 0) {
        recv(client_socket, lost_nos, sizeof(int) * num_lost, 0);
    }

    int received_counts[1000] = {0}; // Track record ke liye ki packet kitni baar aaya

    while (1) {
        packet p;
        int recv_status = recv(client_socket, &p, sizeof(p), 0);
        if (recv_status <= 0) break; 

        received_counts[p.data]++;
        
        printf("--------------------------------------------------------------------\n");

        // Tumhara case 2: Agar packet lost list mein hai aur pehli baar aaya hai
        if (is_lost(p.data, lost_nos, num_lost) && received_counts[p.data] == 1) {
            // Receive karo par ACK mat bhejo, aur time print karo
            printf("Packet %d received at t = %d but dropped (ACK not sent)\n", p.data, p.prop_time);
            continue; 
        }

        // Tumhara case 1 & Case 2 ka second part: Packet aaya aur ACK bhejna hai
        printf("Packet %d received at t = %d, sending ACK\n", p.data, p.prop_time);
        
        ack a;
        a.ack_no = p.data;
        a.rtt = tt + 2 * tp; 
        send(client_socket, &a, sizeof(a), 0);
    }

    close(client_socket);
    close(server_socket);
    return 0;
}