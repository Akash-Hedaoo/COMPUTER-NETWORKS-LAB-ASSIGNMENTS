
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

int p[20], dist[20], visited[20];

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
        close(server_socket);
        return 1;
    }

    if (listen(server_socket, 5) < 0) {
        perror("Listen failed");
        close(server_socket);
        return 1;
    }

    printf("Waiting for the client...\n");

    int client_socket = accept(server_socket, NULL, NULL);

    if (client_socket < 0) {
        perror("Accept failed");
        close(server_socket);
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
        visited[i] = 0;
    }

    // Validate edges; Dijkstra requires non-negative costs.
    for (int i = 0; i < links; i++) {
        if (edges[i].s < 0 || edges[i].s >= routers ||
            edges[i].d < 0 || edges[i].d >= routers ||
            edges[i].cost < 0) {
            printf("Invalid edge or negative cost!\n");
            close(client_socket);
            close(server_socket);
            return 1;
        }
    }

    dist[source] = 0;

    // Dijkstra's algorithm
    for (int cnt = 0; cnt < routers; cnt++) {
        int u = -1;
        int minimum = INF;

        for (int i = 0; i < routers; i++) {
            if (!visited[i] && dist[i] < minimum) {
                minimum = dist[i];
                u = i;
            }
        }

        if (u == -1)
            break;

        visited[u] = 1;

        for (int i = 0; i < links; i++) {
            int from = edges[i].s;
            int to = edges[i].d;
            int cost = edges[i].cost;

            if (from == u && !visited[to] &&
                dist[u] != INF &&
                dist[u] + cost < dist[to]) {
                dist[to] = dist[u] + cost;
                p[to] = u;
            }
        }
    }

    printf("Routing Table (Dijkstra)\n");
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
