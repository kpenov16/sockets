#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define PORT 8080


int main(int arg_c, char const * args[]){

    int sock = 0;
    int val_read;
    struct sockaddr_in server_addr;

    char * hello = "Hello from client";

    char buffer[1024] = {0};

    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){  //AF_INET is IPv4, 
                                                        //SOCK_STREAM is TCP, 
                                                        //protocol is the network layer protocol - IP
        printf("\n error at socket creation\n");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    //convert string 10.10.10.5 into network byte order in server_addr.sin_addr
    //if( inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0){
    if( inet_pton(AF_INET, "192.168.1.33", &server_addr.sin_addr) <= 0){
        printf("\n inet_pton error in converting string IP address into network byte order in server_addr.sin_addr \n");
        return -1;
    }

    if(connect(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0 ){
        printf("\nconnection was not extablished\n");
        return -1;
    }

    send(sock, hello, strlen(hello), 0);
    printf("Hello msg was sent\n");
    val_read = read(sock, buffer, 1024);
    printf("recieved msg: %s\n", buffer);
    return 0;
}




 













