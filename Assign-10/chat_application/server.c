#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>

int main(){
    // CREATING THE SOCKET...
    int server_socket;
    server_socket = socket(AF_INET , SOCK_STREAM , 0);
    if(server_socket <0 ){
        printf("Error creating the socket...\n");
    }

    //SETTING UP THE SERVER ADDRESS
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8080);
    server_address.sin_addr.s_addr = INADDR_ANY;

    //BIND THE SERVER
    int bind_status = bind(server_socket , (struct sockaddr *) &server_address , sizeof(server_address));
    if(bind_status<0){
        printf("Error binding the socket...\n");
    }
    
    //LISTEN ON THE PORT

    int listen_status =  listen(server_socket , 5);

    //yaha par client connect nahi hota...so validation galat hai 

    // if(listen_status<0){
    //     printf("Error connecting with the client...\n");
    // }
    // else{
    //     printf("Client connected successfully...\n");
    // }



    //ACCEPTING THE CONNECTION REQ...

    // waiting for the client
    printf("Waiting for the client ... \n");
    int client_socket = accept(server_socket , NULL , NULL);
    if(client_socket<0){
        printf("Error connecting to client...\n");
    }
    else{
        printf("Client connected successfully...\n\n");
    }

    //INF LOOP FOR CHATTING 

    char message_sent[2000];
    char message_recieved[2000];
    while(1){
        //CUSTOM MESSAGE INPUT
        
        
        // printf("Enter the messages...\n\n");
        printf("SERVER : ");
        fgets(message_sent , sizeof(message_sent) , stdin);
        printf("\n");
    
        //send message to the server 
        int sent_status = send(client_socket , message_sent , sizeof(message_sent) , 0);
        if(sent_status<0){
            printf("Error sending the message ...\n");
            break;
        }

        int recieved_status = recv(client_socket , &message_recieved , sizeof(message_recieved) , 0);
        if(recieved_status<0){
            printf("Error recieving the message...\n");
            break;
        }
        else{
            printf("CLIENT : %s \n" , message_recieved);
                
        }
        

    }



    return 0;
}