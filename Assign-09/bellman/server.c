
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define INF 1000

typedef struct {
    int s;
    int d;
    int cost;
} edge;

int p[20], dist[20];

int main()
{
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket < 0) {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in server_address = {0};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8080);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int bind_status = bind(server_socket,
        (struct sockaddr *)&server_address,
        sizeof(server_address));

    if (bind_status < 0) {
        perror("Bind failed");
        return 1;
    }

    int listen_status = listen(server_socket, 5);

    if (listen_status < 0) {
        perror("Listen failed");
        return 1;
    }

    printf("Waiting for the client...\n");

    int client_socket = accept(server_socket, NULL, NULL);

    if (client_socket < 0) {
        perror("Accept failed");
        return 1;
    }

    printf("Client connected successfully!\n\n");

    int routers, links;
    edge edges[100];

    if (recv(client_socket, &routers, sizeof(routers), 0) != sizeof(routers) ||
        recv(client_socket, &links, sizeof(links), 0) != sizeof(links)) {
        printf("Failed to receive router/link count\n");
        close(client_socket);
        close(server_socket);
        return 1;
    }

    if (routers < 1 || routers > 20 || links < 0 || links > 100) {
        printf("Invalid topology\n");
        close(client_socket);
        close(server_socket);
        return 1;
    }

    if (recv(client_socket, edges, sizeof(edges), 0) != sizeof(edges)) {
        printf("Failed to receive edges\n");
        close(client_socket);
        close(server_socket);
        return 1;
    }

    int source = 0;

    for (int i = 0; i < routers; i++) {
        dist[i] = INF;
        p[i] = -1;
    }

    dist[source] = 0;

    // Bellman-Ford algorithm
    for (int cnt = 0; cnt < routers - 1; cnt++) {
        int changed = 0;

        for (int i = 0; i < links; i++) {
            int u = edges[i].s;
            int v = edges[i].d;
            int cost = edges[i].cost;

            if (u < 0 || u >= routers ||
                v < 0 || v >= routers || cost < 0) {
                printf("Invalid edge data\n");
                close(client_socket);
                close(server_socket);
                return 1;
            }

            if (dist[u] != INF && dist[u] + cost < dist[v]) {
                dist[v] = dist[u] + cost;
                p[v] = u;
                changed = 1;
            }

            
        }

        if (changed == 0)
            break;
    }

    // Negative cycle detection
    for (int i = 0; i < links; i++) {
    int u = edges[i].s;
    int v = edges[i].d;
    int cost = edges[i].cost;

    if (dist[u] != INF && dist[u] + cost < dist[v]) {
        printf("Negative weight cycle detected!\n");
        close(client_socket);
        close(server_socket);
        return 0;
    }
}

    printf("Routing Table (Bellman-Ford)\n");
    printf("Destination\tCost\tParent\n");

    for (int i = 0; i < routers; i++) {
        if (dist[i] == INF)
            printf("%d\t\tINF\t%d\n", i, p[i]);
        else
            printf("%d\t\t%d\t%d\n", i, dist[i], p[i]);
    }

    printf("\nProcessing Complete\n");

    close(client_socket);
    close(server_socket);

    return 0;
}
