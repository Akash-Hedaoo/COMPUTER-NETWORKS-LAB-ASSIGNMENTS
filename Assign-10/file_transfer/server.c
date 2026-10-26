#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 4096

int main(){
    //CREATE A SERVER SOCKET
    int server_socket = socket(AF_INET , SOCK_STREAM , 0);


    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8080);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int bind_status = bind(server_socket , (struct sockaddr *) &server_address , sizeof(server_address));
    if(bind_status<0){
        printf("Error binding the socket...\n");
    }
    
    //LISTEN ON THE PORT

    int listen_status =  listen(server_socket , 5);

    // WAITING FOR THE CLIENT

    printf("Waiting for the client ... \n");
    int client_socket = accept(server_socket , NULL , NULL);
    if(client_socket<0){
        printf("Error connecting to client...\n");
    }
    else{
        printf("Client connected successfully...\n\n");
    }

    size_t file_size;
    char filename[256];

    if(recv(client_socket , &filename , sizeof(filename) , 0)<0){
        perror("Error recieving the filename ");
        return 1;
    }
    filename[sizeof(filename)-1] = '\0';
    if(recv(client_socket , &file_size , sizeof(file_size) , 0)<0){
        perror("Error recieving the filesize");
        return 1;
    }
    // printf("Filename : %s \n" , filename);
    // printf("File size : %s \n" , file_size);

    char output_path[256]="recieved_files/" ;
    strcat(output_path , filename);

    FILE *fp = fopen(output_path , "wb");

    char buffer[BUFFER_SIZE];
    size_t total_size_recieved = 0;
    while(total_size_recieved < file_size){
        size_t remaining_size = (size_t)(file_size - total_size_recieved);
        size_t chunk_size;
        if(remaining_size < sizeof(buffer)){
            chunk_size = remaining_size;
        }
        else chunk_size = sizeof(buffer);

        size_t recieved_status = recv(client_socket , &buffer , chunk_size , 0);
        if(recieved_status<0){
            perror("Error recievinig chunk ");
            return 1;
        }

        fwrite(buffer , 1 , chunk_size , fp);
        total_size_recieved+=chunk_size;

    }
    fclose(fp);
    printf("File recieved successfully..\n");
    printf("Total bytes received: %ld\n", total_size_recieved);

    return 0;
}