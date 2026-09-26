#ifndef DISCOVERY_H
#define DISCOVERY_H

#define DISCOVERY_PORT "8081"     
#define DISCOVERY_CLIENT_PORT "8082"

#define DISCOVERY_PING "LINDRIOD_SHARE_DISCOVERY_REQUEST"
#define DISCOVERY_PONG "LINDRIOD_SHARE_DISCOVERY_ACCEPT"

int discovery_find_peer(char *peer_ip);

int discovery_create_listener(void);

void discovery_respond_to_ping(int udp_sock);

#endif 