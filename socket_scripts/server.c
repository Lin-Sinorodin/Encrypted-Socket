#include <arpa/inet.h> // inet_addr()
#include <strings.h> // bzero()
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFF 100
#define PORT 8888
#define SERVER_IP "192.168.64.11"
#define SOCK_DECRYPT 8           // set decrypt flag
#define SOCK_ENCRYPT 9           // set encrypt flag


int createSocket() {
    int fd = socket(AF_INET, SOCK_STREAM | (1 << SOCK_DECRYPT), 0);
    if (fd == -1) {
        printf("[!] socket creation failed...\n");
        exit(0);
    }
    printf("[+] socket created\n");
    return fd;
}


int connectClient(int fd) {
    int conn;
    struct sockaddr_in servaddr;
    struct sockaddr_in connaddr;

    // initialize server address
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    // Binding newly created socket to given IP and verification
    if ((bind(fd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
        printf("[!] socket bind failed...\n");
        exit(0);
    }
    else
        printf("[+] socket successfully binded..\n");

    // Now server is ready to listen and verification
    if ((listen(fd, 5)) != 0) {
        printf("[!] listen failed...\n");
        exit(0);
    }
    else
        printf("[+] server listening..\n");


    // Accept the data packet from client and verification
    unsigned int len = sizeof(connaddr);
    conn = accept(fd, (struct sockaddr*)&connaddr, &len);
    if (conn < 0) {
        printf("[s] server accept failed...\n");
        exit(0);
    }
    else
        printf("[+] server accept the client\n");

    return conn;
}


void sendMessage(int fd) {
    char msg[] = "Hello from client";
    send(fd, msg, sizeof(msg), 0);
    printf("[+] sent a message to the server: %s\n", msg);
}


void recvMessage(int fd) {
    char buff[BUFF];
    bzero(buff, sizeof(buff));
    recv(fd, buff, sizeof(buff), 0);
    printf("[+] received a message from the server: %s\n", buff);
}


void closeSocket(int fd) {
    close(fd);
    printf("[+] closed socket\n");
}


int main () {
    int fd = createSocket();
    int conn = connectClient(fd);
//    sendMessage(conn);
    recvMessage(conn);
    closeSocket(fd);
    closeSocket(conn);
}