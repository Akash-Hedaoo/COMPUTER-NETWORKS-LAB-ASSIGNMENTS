#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct
{
    int o[4];
    int prefix;
    int need;
} Req;

Req r;
char result[8192], t1[20], t2[20], t3[20], t4[20], sb_msk[20];

int maskOf(int p)
{
    return (p == 0) ? 0 : (-1 << (32 - p)); // -1 = all 1s
}

void toIP(int ip_bin, char *ip_formated)
{
    sprintf(ip_formated, "%d.%d.%d.%d", (ip_bin >> 24) & 255, (ip_bin >> 16) & 255, (ip_bin >> 8) & 255, ip_bin & 255);
}

int main()
{
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in a;
    memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_port = htons(8088);
    a.sin_addr.s_addr = INADDR_ANY;

    bind(srv, (struct sockaddr *)&a, sizeof(a));
    listen(srv, 3);
    printf("Waiting for client...\n");

    int c = accept(srv, NULL, NULL);

    // 1. receive the struct
    recv(c, &r, sizeof(r), MSG_WAITALL);
    printf("Received: %d.%d.%d.%d /%d, subnets=%d\n",
           r.o[0], r.o[1], r.o[2], r.o[3], r.prefix, r.need);

    // // 2. validate
    // for (int i = 0; i < 4; i++)
    //     if (r.o[i] < 0 || r.o[i] > 255)
    //         r.need = 0;

    // if (r.prefix < 0 || r.prefix > 30 || r.need < 1)
    // {
    //     strcpy(result, "Error: Invalid request\n");
    //     send(c, result, strlen(result), 0);
    //     close(c);
    //     close(srv);
    //     return 1;
    // }

    // 3. borrowed bits
    int borrowed = 0, count = 1;
    while (count < r.need && borrowed < 30)
    {
        count *= 2;
        borrowed++;
    }
    int newp = r.prefix + borrowed;

    // if (newp > 30)
    // {
    //     strcpy(result, "Error: Not enough host bits\n");
    //     send(c, result, strlen(result), 0);
    //     close(c);
    //     close(srv);
    //     return 1;
    // }

    // 4. calculate
    int ip = (r.o[0] << 24) | (r.o[1] << 16) | (r.o[2] << 8) | r.o[3];
    int network = ip & maskOf(r.prefix);
    int subnet_mask = maskOf(newp);
    int n = 1 << (32 - newp); // total hosts in each subnet

    toIP(subnet_mask, sb_msk);
    toIP(network, t1);

    // header: build and send
    sprintf(result,
            "Base Network       : %s/%d\n"
            "Subnets Requested  : %d\n"
            "Subnets Created    : %d\n"
            "Borrowed Bits      : %d\n"
            "New Prefix         : /%d\n"
            "Subnet Mask        : %s\n"
            "Hosts per Subnet   : %d\n"
            "------------------------------\n",
            t1, r.prefix, r.need, count, borrowed, newp, sb_msk, n - 2);

    printf("%s", result);
    send(c, result, strlen(result), 0);

    // 5. every subnet: build one, send one
    for (int i = 0; i < count; i++)
    {
        int temp = network + i * n;
        toIP(temp, t1);
        toIP(temp + 1, t2);
        toIP(temp + n - 2, t3);
        toIP(temp + n - 1, t4);

        sprintf(result,
                "Subnet %d:\n"
                "  Network ID : %s/%d\n"
                "  First Host : %s\n"
                "  Last Host  : %s\n"
                "  Broadcast  : %s\n\n",
                i + 1, t1, newp, t2, t3, t4);

        printf("%s", result);
        send(c, result, strlen(result), 0);
    }

    close(c); // tells the client: no more data
    close(srv);
    return 0;
}