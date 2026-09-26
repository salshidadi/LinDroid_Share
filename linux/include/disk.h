#ifndef DISK_H
#define DISK_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h> 

uint64_t disk_get_file_size(const char *filepath);

int disk_open_read(const char *filepath);

int disk_open_write(const char *filepath);

ssize_t disk_read_chunk(int fd, void *buf, size_t count);

int disk_write_chunk(int fd, const void *buf, size_t count);

void disk_close(int fd);

#endif 