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

int main() {
    // 1. CREATE SOCKET
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error creating the socket");
        return 1;
    }

    // 2. ADDRESS DETAILS
    struct sockaddr_in server_address, client_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8080);
    server_address.sin_addr.s_addr = INADDR_ANY;

    // 3. BIND SOCKET
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Error binding the socket");
        return 1;
    }

    // 4. LISTEN
    if (listen(server_socket, 5) < 0) {
        perror("Error listening");
        return 1;
    }
    printf("Server listening on port 8080...\n");

    // 5. ACCEPT CONNECTION
    socklen_t client_len = sizeof(client_address);
    int client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_len);
    if (client_socket < 0) {
        perror("Error accepting connection");
        return 1;
    }
    printf("Client connected successfully.\n");

    // 6. RECEIVE INITIAL DATA
    int tt, tp, no_of_packets;
    if (recv(client_socket, &tt, sizeof(tt), 0) < 0) perror("Error receiving tt");
    if (recv(client_socket, &tp, sizeof(tp), 0) < 0) perror("Error receiving tp");
    if (recv(client_socket, &no_of_packets, sizeof(no_of_packets), 0) < 0) perror("Error receiving no_of_packets");

    printf("Simulation Parameters Received -> Tt: %d, Tp: %d, Packets: %d\n", tt, tp, no_of_packets);

    // 7. SIMULATE RECEIVING AND ACKNOWLEDGING PACKETS
    for (int i = 1; i <= no_of_packets; i++) {
        packet p;
        int recv_status = recv(client_socket, &p, sizeof(p), 0);
        if (recv_status < 0) {
            perror("Error receiving packet");
            return 1;
        }

        printf("--------------------------------------------------------------------\n");
        printf("Packet %d received at server time t = %d\n", p.data, p.prop_time);

        // Prepare ACK
        ack a;
        a.ack_no = p.data;
        
        a.rtt = tt + 2 * tp; 

        printf("Sending ACK %d for Packet %d\n", a.ack_no, p.data);
        
        int send_status = send(client_socket, &a, sizeof(a), 0);
        if (send_status < 0) {
            perror("Error sending ACK");
            return 1;
        }
    }

    printf("--------------------------------------------------------------------\n");
    printf("All packets received. Transmission Completed.\n");

    // 8. CLOSE SOCKETS
    close(client_socket);
    close(server_socket);

    return 0;
}