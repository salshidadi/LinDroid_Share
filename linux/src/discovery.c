#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <poll.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <stdlib.h>

#include "discovery.h"
#include "network.h"

static void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int discovery_create_listener(void) {
    return network_listen(DISCOVERY_PORT, SOCK_DGRAM);
}

void discovery_respond_to_ping(int udp_sock) {
    struct sockaddr_storage their_addr;
    socklen_t addr_len = sizeof their_addr;
    char buffer[64];

    int numbytes = recvfrom(udp_sock, buffer, sizeof buffer - 1, 0,
                            (struct sockaddr *)&their_addr, &addr_len);
    if (numbytes == -1) return;
    
    buffer[numbytes] = '\0'; 

    if (strcmp(buffer, DISCOVERY_PING) == 0) {
        sendto(udp_sock, DISCOVERY_PONG, strlen(DISCOVERY_PONG), 0,
               (struct sockaddr *)&their_addr, addr_len);
    }
}

static int is_local_ip(const char *ip) {
    if (strcmp(ip, "127.0.0.1") == 0) return 1;

    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1) return 0;
    
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_INET) continue;
        
        char local_ip[64];
        inet_ntop(AF_INET, &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr, local_ip, sizeof(local_ip));
        
        if (strcmp(ip, local_ip) == 0) {
            freeifaddrs(ifaddr);
            return 1; 
        }
    }
    freeifaddrs(ifaddr);
    return 0; 
}

int discovery_find_peer(char *peer_ip) {
    int sockfd;
    int broadcast = 1;
    int reuse = 1;

    if ((sockfd = network_listen(DISCOVERY_CLIENT_PORT, SOCK_DGRAM)) == -1) {
            return -1;
        }

    setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1) {
        close(sockfd);
        return -1;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_INET) continue;
        
        if (!(ifa->ifa_flags & IFF_BROADCAST) || ifa->ifa_broadaddr == NULL) continue;

        struct sockaddr_in *bcast_addr = (struct sockaddr_in *)ifa->ifa_broadaddr;
        bcast_addr->sin_port = htons(atoi(DISCOVERY_PORT));
        
        sendto(sockfd, DISCOVERY_PING, strlen(DISCOVERY_PING), 0,
               (struct sockaddr *)bcast_addr, sizeof(struct sockaddr_in));
    }
    
    freeifaddrs(ifaddr);

    struct pollfd pfds[1];
    pfds[0].fd = sockfd;
    pfds[0].events = POLLIN; 
    
    while (1) {
        int num_events = poll(pfds, 1, 3000); 
        
        if (num_events <= 0) {
            close(sockfd);
            return -1;
        }

        struct sockaddr_storage their_addr;
        socklen_t addr_len = sizeof their_addr;
        char buffer[64];

        int numbytes = recvfrom(sockfd, buffer, sizeof buffer - 1, 0,
                                (struct sockaddr *)&their_addr, &addr_len);
        
        buffer[numbytes] = '\0';

        if (strcmp(buffer, DISCOVERY_PONG) == 0) {
            inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), peer_ip, INET6_ADDRSTRLEN);
            
            if (is_local_ip(peer_ip)) {
                continue;
            }

            close(sockfd);
            return 0;
        }
    }

    close(sockfd);
    return -1;
}