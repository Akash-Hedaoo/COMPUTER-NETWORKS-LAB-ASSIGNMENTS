#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main()
{
    int o[4];
    char buf[1024];
    int n;

    printf("Enter IP address: ");
    scanf("%d.%d.%d.%d", &o[0], &o[1], &o[2], &o[3]);

    int s = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in a;
    memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_port = htons(8087);
    a.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(s, (struct sockaddr *)&a, sizeof(a)) < 0)
    {
        perror("Connection failed");
        return 1;
    }

    send(s, o, sizeof(o), 0); // send 4 ints

    printf("\n");
    while ((n = recv(s, buf, sizeof(buf) - 1, 0)) > 0)
    {
        buf[n] = '\0';
        printf("%s", buf);
    }

    close(s);
    return 0;
}

