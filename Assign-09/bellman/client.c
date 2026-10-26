#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define INF 1000

typedef struct{
    int s ;
    int d;
    int cost;
} edge ;

int p[20] , dist[20]={INF};

int main(){
    int client = socket(AF_INET , SOCK_STREAM , 0);
    //address
    struct sockaddr_in client_address;
    client_address.sin_family = AF_INET;
    client_address.sin_port = htons(8080);
    client_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    printf("Connecting to server...\n");
    if(connect(client , (struct sockaddr *)&client_address , sizeof(client_address))<0){
        perror("Error connecting to the server...\n");
    }
    else{
        printf("Server connected successfully...\n");
    }

    int routers ;
    int links;
    int source  = 0;
    printf("Enter no of routers : ");
    scanf("%d",&routers);

    printf("Enter the no links:");
    scanf("%d" , &links);

    send(client , &routers , sizeof(routers) , 0);
    send(client , &links , sizeof(links) , 0);
    printf("Enter the liks in format source , destination , cost :\n");

    edge edges[100];

    for(int i =0 ;i<links ;i++){
        printf("Enter link %d : ", i+1);
        scanf("%d %d %d",&edges[i].s , &edges[i].d , &edges[i].cost);
    }

    send(client , &edges  , sizeof(edges) ,0);

    printf("Network topology sent successfully...\n");




}
