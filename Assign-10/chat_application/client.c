#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<stdlib.h>
#include<arpa/inet.h>

int main(){

    int client_socket;

    //create client_socket
    client_socket = socket(AF_INET, SOCK_STREAM , 0);
    if(client_socket<0){
        printf("Error creating the socket");
    }

    struct sockaddr_in client_address;

    client_address.sin_family = AF_INET;
    client_address.sin_port = htons(8080);
    client_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    //connect socket 
    printf("Connecting to the server....\n");

    int connection_status = connect(client_socket , (struct sockaddr *) &client_address , sizeof(client_address));
    if(connection_status<0){
        printf("Error in socket connection...\n");
        close(client_socket);
        return 1;
    }else{
        printf("Connected successfully...\n");

    }

    //INF LOOP FOR CHATTING 
    char message_sent[2000];
    char message_recieved[2000];
    while(1){
        //recieving the message from the client ; 
        int recieved_status = recv(client_socket , &message_recieved , sizeof(message_recieved) , 0 );
    
        if(recieved_status<0){
            printf("Error : There was a error recieving the message...\n");
            break;
        }
        else{
            printf("SERVER : %s \n" , message_recieved);            
        }
        printf("CLIENT : ");
        fgets(message_sent , sizeof(message_sent) , stdin);
        printf("\n");
        int sent_status = send(client_socket , &message_sent , sizeof(message_sent) , 0);
        if(sent_status<0){
            printf("Error sending the message ...\n");
            break;
        }


    }




    return 0;
}