#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define PORT 8080

int main(int args_c, char const * args[]){
    int server_fd;
    int new_socket;
    int val_read;
    struct sockaddr_in address;
    int opt = 1;
    int addr_len = sizeof(address);
    char buffer[1024] = {0};
    char * hello = "Hello from server";

    //create socket file descriptor 
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0))==0){   //AF_INET is IPv4, 
                                                            //SOCK_STREAM is TCP, 
                                                            //protocol is the network layer protocol - IP
        perror("failed creating server socket");
        exit(EXIT_FAILURE);
    }

    //assosiate port to the socket 
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons(PORT); //htons converts from host byte order 
                                    //to network byte order (Big endian)

    if( bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0 ){ //assining a name to socket
        perror("binding socket to address failed");
        exit(EXIT_FAILURE);
    }

    if(listen(server_fd, 3) < 0){ //mark the socket as recieving socket on accept
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    if((new_socket = accept(server_fd, 
                            (struct sockaddr *) &address, 
                            (socklen_t *) &addr_len)) < 0 ){
        perror("accept failed");
        exit(EXIT_FAILURE);
    }

    val_read = read(new_socket, buffer, 1024);
    printf("%s\n", buffer);
    send(new_socket, hello, strlen(hello), 0);
    printf("Hello msg sent\n");
    return 0;
}












