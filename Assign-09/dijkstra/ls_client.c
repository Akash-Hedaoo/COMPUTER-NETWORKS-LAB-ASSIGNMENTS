#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define MAX_VERTICES 100

struct LS_Request {
    int vertices;
    int graph[MAX_VERTICES][MAX_VERTICES];
    int source_router;
};

int main(void) {
    int sock;
    struct sockaddr_in server_addr;
    struct LS_Request req;
    char ack;

    /* Gather Network Topology */
    printf("Enter number of routers (1-%d): ", MAX_VERTICES);
    if (scanf("%d", &req.vertices) != 1 || req.vertices < 1 || req.vertices > MAX_VERTICES) {
        fprintf(stderr, "Invalid number of routers.\n");
        return 1;
    }

    printf("Enter the adjacency matrix (-1 means no link, 0 on diagonal):\n");
    for (int i = 0; i < req.vertices; i++) {
        for (int j = 0; j < req.vertices; j++) {
            if (scanf("%d", &req.graph[i][j]) != 1 || req.graph[i][j] < -1) {
                fprintf(stderr, "Invalid link cost. Costs must be >= 0 or -1.\n");
                return 1;
            }
        }
    }

    printf("Enter source router (0-%d): ", req.vertices - 1);
    if (scanf("%d", &req.source_router) != 1 || req.source_router < 0 || req.source_router >= req.vertices) {
        fprintf(stderr, "Invalid source router.\n");
        return 1;
    }

    /* Socket Creation */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation error");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return 1;
    }

    /* Send Data to Server */
    send(sock, &req, sizeof(req), 0);

    /* Wait for server acknowledgment */
    recv(sock, &ack, sizeof(ack), 0);
    
    printf("\nAdjacency matrix sent successfully!\n");
    printf("Check the server console for the calculated OSPF routing table.\n");

    close(sock);
    return 0;
}