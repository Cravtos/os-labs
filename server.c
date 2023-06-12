// used as reference: https://beej.us/guide/bgnet/html/split/system-calls-or-bust.html

// TODO: handle select() erronious ready-to-read with O_NONBLOCK

#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <string.h>
#include <stddef.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#define PORT "31337"
#define MAX_PENDING 50

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main() {
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int rv = 0;
    struct addrinfo *ai, *p;
    getaddrinfo(NULL, PORT, &hints, &ai);
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    int sockfd = 0;
    for (p = ai; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd < 0) {
            continue;
        }

        int yes = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) < 0) {
            close(sockfd);
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai);

    if (listen(sockfd, MAX_PENDING) == -1) {
        perror("listen");
        exit(1);
    }

    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof their_addr;
    char buf[32];
    for (int i = 0; i < 5; i++) {
        int fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
        int n = recv(fd, buf, sizeof buf - 1, 0);
        if (n == -1) {
            perror("recv");
            goto next;
        }
        buf[n] = '\0';
        send(fd, buf, strlen(buf), 0);
next:
        close(fd);
    }

    close(sockfd);
}
