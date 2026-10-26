#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 4096


int main(){

    //CREATING SOCKET;
    int client_socket = socket(AF_INET , SOCK_STREAM , 0);

    if(client_socket<0){
        perror("Error creating socket");
    }

    //CLIENT ADDRESS
    struct sockaddr_in client_address;
    client_address.sin_family = AF_INET;
    client_address.sin_port = htons(8080);
    client_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    //CONNECTING TO SERVER
    printf("Connecting to server...\n");
    if(connect(client_socket , (struct sockaddr *)&client_address , sizeof(client_address))<0){
        perror("Error connecting to the server...\n");
    }
    else{
        printf("Server connected successfully...\n");
    }

    //INPUT FILE PATH
    char filepath[256];

    printf("Enter the file path : ");
    fgets(filepath , sizeof(filepath) , stdin);
    //remove newline 
    filepath[strcspn(filepath , "\n")] = '\0';

    FILE *fp = fopen(filepath , "rb");

    if(fp == NULL){
        perror("File open failed");
        return 1;
    }

    

    //extracting the filename from the path;
    char filename[256];
    int pathlen = strlen(filepath);
    int i = pathlen-1;
    while(i>=0 && filepath[i]!='/'){
        i--;
    }
    int ptr = i+1;
    int nameptr = 0;
    while(filepath[ptr]!='\0'){
        filename[nameptr++]=filepath[ptr++];
    }
    filename[nameptr] = '\0';
    printf("Filename: %s\n", filename);
    //file size extraction
    fseek(fp , 0 , SEEK_END);
    long file_size = ftell(fp);
    rewind(fp);

    //SENDING FILE NAME AND SIZE
    if(send(client_socket , filename , sizeof(filename) , 0)<0){
        perror("Error sending the file name...\n");
        return 1;
    }
    if(send(client_socket , &file_size , sizeof(file_size ) , 0)<0){
        perror("Error sending the file size...\n");
        return 1;

    }


    //SENDING THE DATA USING CHUNKING

    char buffer[BUFFER_SIZE];
    size_t bytes_read;

    size_t size_total_sent =0;

    while((bytes_read = fread(buffer , 1 , sizeof(buffer) , fp))>0){
        if(send(client_socket , buffer , bytes_read ,0)<0){
            perror("Error sending the file...\n");
            return 1;
        }
        size_total_sent+=bytes_read;
    }

    return 0;
}