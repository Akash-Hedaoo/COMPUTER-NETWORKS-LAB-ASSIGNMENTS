#include <stdio.h>

int main(void)
{
    int octet[4];
    int first;
    char class_name;
    char nid[16] = "N/A";
    char hid[16] = "N/A";
    const char *network_count = "N/A";
    const char *host_count = "N/A";
    char lba[16] = "N/A";
    char dba[16] = "N/A";

    printf("Enter an IPv4 address (e.g., 192.168.1.1): ");
    if (scanf("%d.%d.%d.%d", &octet[0], &octet[1], &octet[2], &octet[3]) != 4) {
        printf("Invalid IP address format.\n");
        return 1;
    }

    for (int i = 0; i < 4; i++) {
        if (octet[i] < 0 || octet[i] > 255) {
            printf("Invalid IP address. Each octet must be between 0 and 255.\n");
            return 1;
        }
    }

    first = octet[0];

    if (first == 0) {
        class_name = '-';
    } else if (first <= 127) {
        class_name = 'A';
        snprintf(nid, sizeof(nid), "%d.0.0.0", octet[0]);
        snprintf(hid, sizeof(hid), "0.%d.%d.%d", octet[1], octet[2], octet[3]);
        network_count = "126";
        host_count = "16,777,214";
        snprintf(lba, sizeof(lba), "%d.0.0.1", octet[0]);
        snprintf(dba, sizeof(dba), "%d.255.255.255", octet[0]);
    } else if (first <= 191) {
        class_name = 'B';
        snprintf(nid, sizeof(nid), "%d.%d.0.0", octet[0], octet[1]);
        snprintf(hid, sizeof(hid), "0.0.%d.%d", octet[2], octet[3]);
        network_count = "16,384";
        host_count = "65,534";
        snprintf(lba, sizeof(lba), "%d.%d.0.1", octet[0], octet[1]);
        snprintf(dba, sizeof(dba), "%d.%d.255.255", octet[0], octet[1]);
    } else if (first <= 223) {
        class_name = 'C';
        snprintf(nid, sizeof(nid), "%d.%d.%d.0", octet[0], octet[1], octet[2]);
        snprintf(hid, sizeof(hid), "0.0.0.%d", octet[3]);
        network_count = "2,097,152";
        host_count = "254";
        snprintf(lba, sizeof(lba), "%d.%d.%d.1", octet[0], octet[1], octet[2]);
        snprintf(dba, sizeof(dba), "%d.%d.%d.255", octet[0], octet[1], octet[2]);
    } else if (first <= 239) {
        class_name = 'D';
    } else {
        class_name = 'E';
    }

    printf("\n+-------+----------------+----------------+----------------+----------------+----------------------+-----------------+-------------------+\n");
    printf("| Class | NID            | HID            | No. of Network | No. of Host    | LBA                  | DBA             | IP Type           |\n");
    printf("+-------+----------------+----------------+----------------+----------------+----------------------+-----------------+-------------------+\n");
    printf("| %-5c | %-14s | %-14s | %-14s | %-14s | %-20s | %-15s | %-17s |\n",
           class_name, nid, hid, network_count, host_count, lba, dba,
           first >= 224 && first <= 239 ? "Multicast" :
           first >= 240 ? "Reserved" :
           first == 127 ? "Loopback" :
           first == 0 && octet[1] == 0 && octet[2] == 0 && octet[3] == 0 ? "Unspecified" :
           first == 10 || (first == 172 && octet[1] >= 16 && octet[1] <= 31) ||
           (first == 192 && octet[1] == 168) ? "Private" : "Public");
    printf("+-------+----------------+----------------+----------------+----------------+----------------------+-----------------+-------------------+\n");

    return 0;
}