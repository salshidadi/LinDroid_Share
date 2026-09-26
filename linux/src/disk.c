#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "disk.h"

uint64_t disk_get_file_size(const char *filepath) {
    struct stat file_stat;
    if (stat(filepath, &file_stat) < 0) {
        return 0; 
    }
    return (uint64_t)file_stat.st_size;
}

int disk_open_read(const char *filepath) {
    return open(filepath, O_RDONLY);
}

int disk_open_write(const char *filepath) {
    return open(filepath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
}

ssize_t disk_read_chunk(int fd, void *buf, size_t count) {
    return read(fd, buf, count);
}

int disk_write_chunk(int fd, const void *buf, size_t count) {
    size_t total = 0;
    size_t bytesleft = count;
    ssize_t n;
    const char *pbuf = buf; 

    while (total < count) {
        n = write(fd, pbuf + total, bytesleft);
        if (n <= 0) {
            return -1; 
        }
        total += n;
        bytesleft -= n;
    }

    return 0;
}

void disk_close(int fd) {
    close(fd);
}