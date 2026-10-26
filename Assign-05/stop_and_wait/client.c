#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<string.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>

typedef struct{
    int data ;
    int prop_time;
}packet;

typedef struct {
    int ack_no ;
    int rtt;
}ack ;

int main(){
    //CREATE SOCKET
    int client_socket = socket(AF_INET , SOCK_STREAM , 0);
    if(client_socket<0){
        perror("Error creating the socket ");
        return 1;
    }

    //ADDRESS DETAILS
    struct sockaddr_in client_address;
    client_address.sin_family = AF_INET;
    client_address.sin_port = htons(8080);
    client_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    // CONNECTING 

    if(connect(client_socket , (struct sockaddr *)&client_address , sizeof(client_address))<0){
        perror("Error connecting the socket ");
        return 1;
    }

    int tt , tp , no_of_packets;
    printf("Enter Tt : ");
    scanf("%d" , &tt);
    printf("Enter Tp : ");
    scanf("%d" , &tp);
    printf("Enter no of packets : ");
    scanf("%d" , &no_of_packets);

    //sending the var to server also 
    if(send(client_socket , &tt , sizeof(tt) , 0)<0){
        perror("Error sending the basic data :");
    }
    if(send(client_socket , &tp , sizeof(tp) , 0)<0){
        perror("Error sending the basic data :");
    }
    if(send(client_socket , &no_of_packets , sizeof(no_of_packets) , 0)<0){
        perror("Error sending the basic data :");
    }

    int rtt_calculated = 2*(tt + tp);
    int to_calculated = 3*rtt_calculated;
    printf("Transmission time = %d \n" ,tt );
    printf("Propagation time = %d \n" ,tp);
    printf("Round trip time 2(tt + tp) =  %d \n" , rtt_calculated );
    printf("Timeout time 3(Rtt) = %d \n" , to_calculated );
    int curr_time  = 0 ;
    int recieved_time = 0 ;
    int curr_packet = 1;

    while(curr_packet <= no_of_packets){

        curr_time+=tt;

        printf("--------------------------------------------------------------------\n");
        printf("Packet %d sent at t = %d \n" , curr_packet , curr_time);

        packet p = {curr_packet , curr_time + tp};
        int send_status = send(client_socket , &p , sizeof(p) , 0);
        if(send_status<0){
            perror("Error sending the packet");
            return 1;
        }

        ack a;
        int recieved_status = recv(client_socket, &a , sizeof(a) ,0);
        if(recieved_status<0){
            perror("Error recieving the ack");
            return 1;
        }
        printf("Packet ack recieved at t = %d\n" , a.rtt);
        curr_time +=a.rtt;    
        
        curr_packet++;
        
    }
    printf("Transmission Completed \n");
    printf("Total time taken n(2(tt + tp)) = %d \n" , no_of_packets*rtt_calculated);


    return 0;
}