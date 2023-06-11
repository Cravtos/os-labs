// TODO: do not forget about error handling (https://beej.us/guide/bgnet/html/split/man-pages.html#errnoman)

#include <sys/socket.h>
#include <string.h>
#include <stddef.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#define PORT "31337"
#define MAX_PENDING 50


int main() {
    struct addrinfo hints, *res;
    int sockfd;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, PORT, &hints, &res);
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1) {
        perror("socket");
        return -1;
    }

    bind(sockfd, res->ai_addr, res->ai_addrlen);

    if (listen(sockfd, MAX_PENDING) == -1) {
        perror("listen");
        return -1;
    }

    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof their_addr;
    char* hello_msg = "Hello!\n";
    for (int i = 0; i < 5; i++) {
        int fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
        send(fd, hello_msg, strlen(hello_msg), 0);
        close(fd);
    }

    close(sockfd);
}
