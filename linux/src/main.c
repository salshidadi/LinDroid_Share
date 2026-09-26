#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h> 
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include "network.h"
#include "protocol.h"
#include "disk.h"
#include "discovery.h"

#define CHUNK_SIZE 4096

void daemonize() {
    pid_t pid;

    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS); 

    if (setsid() < 0) exit(EXIT_FAILURE);

    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    umask(0);
    chdir("/");

    int fd = open("/dev/null", O_RDWR);
    if (fd >= 0) {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if (fd > STDERR_FILENO) {
            close(fd);
        }
    }
}

void run_daemon() {    
    daemonize();

    int tcp_fd = network_listen(DEFAULT_PORT, SOCK_STREAM);
    if (tcp_fd < 0) {
        return;
    }

    int udp_fd = discovery_create_listener();
    if (udp_fd < 0) {
        network_close(tcp_fd);
        return;
    }

    struct pollfd pfds[2];
    
    pfds[0].fd = tcp_fd;
    pfds[0].events = POLLIN; 
    
    pfds[1].fd = udp_fd;
    pfds[1].events = POLLIN; 

    while (1) {
        int num_events = poll(pfds, 2, -1); 
        if (num_events < 0) {
            break;
        }

        if (pfds[1].revents & POLLIN) {
            printf("someone send ping!\n");
            discovery_respond_to_ping(udp_fd);
        }

        if (pfds[0].revents & POLLIN) {
            int client_fd = network_accept(tcp_fd);
            if (client_fd < 0) continue;

            struct file_header header;
            if (protocol_recv_header(client_fd, &header) < 0) {
                network_close(client_fd);
                continue; 
            }

            const char *save_dir = "/home/salman/Desktop/LinDriod";
            mkdir(save_dir, 0777); 

            char full_path[512];
            snprintf(full_path, sizeof(full_path), "%s/%s", save_dir, header.filename);

            int file_fd = disk_open_write(full_path);
            if (file_fd < 0) {
                network_close(client_fd);
                continue;
            }
            
            uint8_t buffer[CHUNK_SIZE];
            uint64_t bytes_received = 0;

            while (bytes_received < header.file_size) {
                size_t to_read = CHUNK_SIZE;
                if (header.file_size - bytes_received < CHUNK_SIZE) {
                    to_read = header.file_size - bytes_received;
                }

                if (network_recv_exact(client_fd, buffer, to_read) < 0) break;
                if (disk_write_chunk(file_fd, buffer, to_read) < 0) break;

                bytes_received += to_read;
            }

            disk_close(file_fd);
            network_close(client_fd);
        }
    }
    
    network_close(tcp_fd);
    network_close(udp_fd);
}

void run_sender(const char *filepath) {
    char discovered_ip[64];

    if (discovery_find_peer(discovered_ip) < 0) {
            return;
    }

    uint64_t file_size = disk_get_file_size(filepath);
    if (file_size == 0) {
        return;
    }

    const char *filename = strrchr(filepath, '/');
    filename = (filename) ? filename + 1 : filepath;

    struct file_header header;
    header.magic = PROTOCOL_MAGIC;
    header.file_size = file_size;
    strncpy(header.filename, filename, MAX_FILENAME_LEN - 1);
    header.filename[MAX_FILENAME_LEN - 1] = '\0'; 

    int socket_fd = network_connect(discovered_ip, DEFAULT_PORT);
    if (socket_fd < 0) {
        return;
    }

    if (protocol_send_header(socket_fd, &header) < 0) {
        network_close(socket_fd); 
        return;
    }

    int file_fd = disk_open_read(filepath);
    if (file_fd < 0) {
        network_close(socket_fd);
        return;
    }

    uint8_t buffer[CHUNK_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = disk_read_chunk(file_fd, buffer, CHUNK_SIZE)) > 0) {
        if (network_send_all(socket_fd, buffer, bytes_read) < 0) {
            break;
        }
    }

    disk_close(file_fd);
    network_close(socket_fd);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage:\n");
        printf("  Daemon: %s --daemon\n", argv[0]);
        printf("  Send: %s --send \n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "--daemon") == 0) {
        run_daemon();
    } else if (strcmp(argv[1], "--send") == 0) {
        run_sender(argv[2]); 
    } else {
        printf("Invalid command.\n");
    }

    return 0;
}