#include <arpa/inet.h> // inet_addr()
#include <strings.h> // bzero()
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFF 100
#define PORT 8888
#define SERVER_IP "192.168.64.11"
#define SOCK_ENCRYPT 9           // set encrypt flag


int createSocket() {
    int fd = socket(AF_INET, SOCK_STREAM | (1 << SOCK_ENCRYPT), 0);
    if (fd == -1) {
        printf("[!] socket creation failed...\n");
        exit(0);
    }
    printf("[+] socket created\n");
    return fd;
}


void connectServer(int fd) {
    struct sockaddr_in servaddr;

    // initialize server address
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    servaddr.sin_port = htons(PORT);

    // connect the client socket to server socket
    if (connect(fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        printf("[!] connection with the server failed...\n");
        close(fd);
        exit(0);
    }
    printf("[+] connected to the server\n");
}


void sendMessage(int fd) {
    char msg[] = "Hello from client";
    send(fd, msg, sizeof(msg), 0);
    printf("[+] sent a message to the server: %s\n", msg);
}


void recvMessage(int fd) {
    char buff[BUFF];
    bzero(buff, sizeof(buff));
    read(fd, buff, sizeof(buff));
    printf("[+] received a message from the server: %s\n", buff);
}


void closeSocket(int fd) {
    close(fd);
    printf("[+] closed socket\n");
}


int main () {
    int fd = createSocket();
    connectServer(fd);
    sendMessage(fd);
//    recvMessage(fd);
    closeSocket(fd);
}