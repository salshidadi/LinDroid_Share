#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#define PROTOCOL_MAGIC 0x45566773 
#define MAX_FILENAME_LEN 256

struct file_header {
    uint32_t magic;                  
    uint64_t file_size;               
    char filename[MAX_FILENAME_LEN];  
};

int protocol_send_header(int socket_fd, const struct file_header *header);
int protocol_recv_header(int socket_fd, struct file_header *header);

#endif 