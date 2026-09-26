#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

#include "network.h"


int network_listen(const char *port, int socktype) {
    struct addrinfo hints, *servinfo, *p;
    int sockfd;
    int yes = 1;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;     
    hints.ai_socktype = socktype;   
    hints.ai_flags = AI_PASSIVE;    

    if (getaddrinfo(NULL, port, &hints, &servinfo) != 0) {
        return -1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            close(sockfd);
            return -1;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            continue;
        }
        break; 
    }

    freeaddrinfo(servinfo); 

    if (p == NULL) {
        return -1;
    }

    if (socktype == SOCK_STREAM) {
        if (listen(sockfd, 10) == -1) {
            close(sockfd);
            return -1;
        }
    }

    return sockfd;
}

int network_accept(int listen_fd) {
    struct sockaddr_storage their_addr; 
    socklen_t sin_size = sizeof their_addr;
    
    int new_fd = accept(listen_fd, (struct sockaddr *)&their_addr, &sin_size);
    
    return new_fd;
}

int network_connect(const char *ip, const char *port) {
    struct addrinfo hints, *servinfo, *p;
    int sockfd;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;   
    hints.ai_socktype = SOCK_STREAM; 

    if (getaddrinfo(ip, port, &hints, &servinfo) != 0) {
        return -1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd); 
            continue;
        }
        break; 
    }

    freeaddrinfo(servinfo);

    if (p == NULL) {
        return -1;
    }

    return sockfd;
}

int network_send_all(int fd, const void *buf, size_t len) {
    size_t total = 0;       
    size_t bytesleft = len;  
    ssize_t n;
    const char *pbuf = buf;

    while (total < len) {
        n = send(fd, pbuf + total, bytesleft, 0);
        if (n == -1) {
            break; 
        }
        total += n;
        bytesleft -= n;
    }

    return n == -1 ? -1 : 0; 
}

int network_recv_exact(int fd, void *buf, size_t len) {
    size_t total = 0;
    size_t bytesleft = len;
    ssize_t n;
    char *pbuf = buf;

    while (total < len) {
        n = recv(fd, pbuf + total, bytesleft, 0);

        if (n <= 0) {
            return -1; 
        }
        total += n;
        bytesleft -= n;
    }

    return 0;
}

void network_close(int fd) {
    close(fd); 
}