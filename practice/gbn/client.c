#include<stdio.h>
#include<arpa/inet.h>
#include<string.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include<stdlib.h>


void printwindow(int base , int windowsize){
    printf("window : [");
    for(int i=base ; i<=base+ windowsize ;i++){
        printf("%d " , i);
    }
    printf("] \n");
}

int main(){

    //create socket 
    
    int socket = socket(AF_INET , SOCK_STREAM , 0);
    
    struct sockaddr_in sockadd ;
    sockadd.sin_family = AF_INET;
    sockadd.sin_port=htons(8080);
    sockadd.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(socket , (struct sockaddr*)&sockadd , sizeof(sockadd) );

    int ws  , nthlost , noofpackets;
    printf("ws , nth lost , no of packet : ");
    scanf("%d %d %d " , &ws , &nthlost , &noofpackets);

    int base = 1;    
    int packetsend = 0;
    while(base<=noofpackets && packetsend<=noofpackets){

        //current window 
        printwindow(base , ws);
        //send the current window 
        for(int i = base ; i < base + ws -1 ;i++){
            send(socket , &i , sizeof(i) , 0);
            packetsend++;
        }

        //recieve ack for the packets sent;
        int recv_cnt = 0 ;
        int lst_recieved = 0;
        for(int i = 0 ; i<packetsend ;i++){
            int ackno = -1;
            recv(socket ,&ackno , sizeof(ackno) , 0);

            //ack recieved 
            if(ackno>=1 && ackno<=noofpackets){
                printf("Ack recieved for %d packet \n" , ackno);
                //sending the next packet by expanding the window 
                int packet_no = base+ws+i+1;
                lst_recieved++;
                send(socket , &packet_no ,  sizeof(packet_no) , 0);
                packetsend++;


            }
            else{
                //out of order
                if(ackno==-2){
                    printf("Out of order packet\n");
                }
                else{
                    //lost packet simulated
                    printf("Packet lost \n");
                }
                
                
            }

        }
        base += lst_recieved;
        



    }
    

    return 0;

}