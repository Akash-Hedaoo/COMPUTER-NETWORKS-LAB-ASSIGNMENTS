#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct
{
    int o[4]; // ip octets
    int prefix;
    int need; // subnets needed
} Req;        // must be same as server

int main()
{
    Req r;
    char result[8192];

    printf("Enter network address: ");
    scanf("%d.%d.%d.%d", &r.o[0], &r.o[1], &r.o[2], &r.o[3]);
    printf("Enter prefix (0-30): ");
    scanf("%d", &r.prefix);
    printf("Enter number of subnets: ");
    scanf("%d", &r.need);

    int s = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in a;
    memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_port = htons(8088);
    a.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(s, (struct sockaddr *)&a, sizeof(a)) < 0)
    {
        perror("Connection failed");
        return 1;
    }

    send(s, &r, sizeof(r), 0); // send the whole struct

    char buf[1024];
    int n;
    while ((n = recv(s, buf, sizeof(buf) - 1, 0)) > 0)
    {
        buf[n] = '\0';     // end this piece
        printf("%s", buf); // print it directly
    }
    close(s);
    return 0;
}