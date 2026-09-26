#ifndef NETWORK_H
#define NETWORK_H

#include <stddef.h>

#define DEFAULT_PORT "8080"

int network_listen(const char *port, int socktype);

int network_accept(int listen_fd);

int network_connect(const char *ip, const char *port);

int network_send_all(int fd, const void *buf, size_t len);

int network_recv_exact(int fd, void *buf, size_t len);

void network_close(int fd);

#endif 