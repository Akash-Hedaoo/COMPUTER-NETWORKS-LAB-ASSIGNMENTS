#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define MAX_VERTICES 100

struct LS_Request {
    int vertices;
    int graph[MAX_VERTICES][MAX_VERTICES];
    int source_router;
};

static void print_path(int parent[], int vertex) {
    if (parent[vertex] == -1) {
        printf("%d", vertex);
        return;
    }
    print_path(parent, parent[vertex]);
    printf(" -> %d", vertex);
}

void handle_client(int client_fd) {
    struct LS_Request req;
    char ack = 'K';
    int distance[MAX_VERTICES];
    int parent[MAX_VERTICES];
    int visited[MAX_VERTICES] = {0};

    /* Use MSG_WAITALL to ensure we receive the full 40KB struct over TCP */
    if (recv(client_fd, &req, sizeof(req), MSG_WAITALL) <= 0) {
        close(client_fd);
        return;
    }

    printf("\n--- Processing Link State Routing ---\n");
    printf("Total Routers: %d\n", req.vertices);
    printf("Source Router: %d\n\n", req.source_router);

    /* Initialize Dijkstra arrays */
    for (int i = 0; i < req.vertices; i++) {
        distance[i] = INT_MAX;
        parent[i] = -1;
    }
    distance[req.source_router] = 0;

    /* Dijkstra's Algorithm */
    for (int step = 0; step < req.vertices; step++) {
        int current = -1;

        for (int i = 0; i < req.vertices; i++) {
            if (!visited[i] && distance[i] != INT_MAX &&
                (current == -1 || distance[i] < distance[current])) {
                current = i;
            }
        }

        if (current == -1) {
            break;
        }
        visited[current] = 1;

        for (int neighbor = 0; neighbor < req.vertices; neighbor++) {
            int cost = req.graph[current][neighbor];
            if (!visited[neighbor] && cost >= 0 && distance[current] <= INT_MAX - cost &&
                distance[current] + cost < distance[neighbor]) {
                distance[neighbor] = distance[current] + cost;
                parent[neighbor] = current;
            }
        }
    }

    /* Print routing table on server console */
    printf("Link-state routing table (Dijkstra/OSPF model)\n");
    printf("Destination\tCost\tPath\n");
    for (int i = 0; i < req.vertices; i++) {
        printf("%d\t\t", i);
        if (distance[i] == INT_MAX) {
            printf("INF\tUnreachable\n");
        } else {
            printf("%d\t", distance[i]);
            print_path(parent, i);
            putchar('\n');
        }
    }
    printf("\n--- Finished Processing ---\n\n");

    /* Send acknowledgment back to client */
    send(client_fd, &ack, sizeof(ack), 0);
    close(client_fd);
}

int main(void) {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int opt = 1;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_fd, 5);

    printf("Link State Server listening on port %d...\n", PORT);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd >= 0) {
            handle_client(client_fd);
        }
    }
    return 0;
}