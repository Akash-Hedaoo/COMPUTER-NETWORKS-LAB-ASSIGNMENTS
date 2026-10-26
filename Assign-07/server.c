#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

char result[500], t1[20], t2[20];

void toIP(int num, char *str)
{
    sprintf(str, "%d.%d.%d.%d", (num >> 24) & 255, (num >> 16) & 255, (num >> 8) & 255, num & 255);
}

int main()
{
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in a;
    memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_port = htons(8087);
    a.sin_addr.s_addr = INADDR_ANY;

    bind(srv, (struct sockaddr *)&a, sizeof(a));
    listen(srv, 3);
    int c = accept(srv, NULL, NULL);

    // 1. receive 4 octets
    int o[4], i;
    recv(c, o, sizeof(o), MSG_WAITALL);

    // 2. validate
    for (i = 0; i < 4; i++)
        if (o[i] < 0 || o[i] > 255)
        {
            strcpy(result, "Invalid IP Address\n");
            send(c, result, strlen(result), 0);
            close(c);
            close(srv);
            return 1;
        }

    // 3. IP as one number
    int ip = (o[0] << 24) | (o[1] << 16) | (o[2] << 8) | o[3];

    // 4. class = count of leading 1s: 0->A, 1->B, 2->C, 3->D, 4->E
    int ones = 0;
    while (ones < 4 && ((ip >> (31 - ones)) & 1))
        ones++;
    int len = sprintf(result, "Class      : %c\n", 'A' + ones);

    // 5. A, B, C have NID / HID, D and E do not
    if (ones <= 2)
    {
        int nid = 8 * (ones + 1); // 8, 16, 24
        int hid = 32 - nid;
        int networks = 1 << (nid - (ones + 1)); // nid minus fixed class bits
        if (ones == 0)
            networks -= 2; // class A: 0 and 127 reserved
        int hosts = (1 << hid) - 2;

        int net = ip & (-1 << hid);     // network address
        toIP(net + 1, t1);              // LBA = first host
        toIP(net + (1 << hid) - 2, t2); // DBA = last host

        sprintf(result + len,
                "NID Bits   : %d\n"
                "HID Bits   : %d\n"
                "Networks   : %d\n"
                "Hosts      : %d\n"
                "LBA        : %s\n"
                "DBA        : %s\n",
                nid, hid, networks, hosts, t1, t2);
    }
    else if (ones == 3)
        sprintf(result + len, "Used for Multicasting\n");
    else
        sprintf(result + len, "Used for Experimental/Reserved purpose\n");

    printf("%s", result);
    send(c, result, strlen(result), 0);

    close(c);
    close(srv);
    return 0;
}