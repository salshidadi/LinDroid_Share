#include <stdio.h>

#include "protocol.h"
#include "network.h"


int protocol_send_header(int socket_fd, const struct file_header *header) {
    if (network_send_all(socket_fd, header, sizeof(struct file_header)) < 0) {
        return -1;
    }
    return 0;
}

int protocol_recv_header(int socket_fd, struct file_header *header) {
    if (network_recv_exact(socket_fd, header, sizeof(struct file_header)) < 0) {
        return -1;
    }

    if (header->magic != PROTOCOL_MAGIC) {
        return -1;
    }

    return 0;
}