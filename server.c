#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define DEFAULT_HOST "::1"
#define DEFAULT_PORT 12345
#define BACKLOG 128  // listenの上限値になって（しまって）いる値
#define BUFFER_SIZE 1500  // イーサネットのMTU

int main(int argc, char **argv){
    // prepare socket
    int socket_fd;
    socket_fd = socket(PF_INET6, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    };

    // prepare address struct
    struct sockaddr_in6 server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_len = sizeof(struct sockaddr_in6);
    server_addr.sin6_family = AF_INET6;
    int port = DEFAULT_PORT;
    server_addr.sin6_port = htons(port);

    // try to set host address
    char *host = DEFAULT_HOST;
    if (inet_pton(AF_INET6, host, &server_addr.sin6_addr) <= 0) {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }
    
    // bind address to socket
    if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(socket_fd);
        exit(EXIT_FAILURE);
    } 
    printf("Server is running on [%s]:%d\n", host, port);

    // start listening
    if (listen(socket_fd, BACKLOG) < 0) {
        perror("listen");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    // accept connection
    struct sockaddr_in6 client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_fd < 0) {
        perror("accept");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }
    puts("Client connected");

    // echo back client's message
    char buffer[BUFFER_SIZE];
    while (1) {
        // receive message
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes_received = read(client_fd, buffer, sizeof(buffer) - 1);
        if (bytes_received < 0) {
            if (errno == EAGAIN || errno == EINTR) {
                continue;
            } else {
                perror("read");
                close(socket_fd);
                exit(EXIT_FAILURE);
            }
        } else if (bytes_received == 0) {
            puts("connection finished.");
            break;
        }
        buffer[bytes_received] = '\0';

        // return message
        ssize_t total_bytes_sent = 0;
        // loop until all received messages are sent back
        while (total_bytes_sent < bytes_received){
            ssize_t bytes_sent = write(client_fd, buffer, bytes_received);
            if (bytes_sent  < 0) {
                if (errno == EAGAIN || errno == EINTR) {
                    continue;
                } else {
                    perror("write");
                    close(socket_fd);
                    exit(EXIT_FAILURE);
                }
            }
            total_bytes_sent += bytes_sent;
            printf("received and echoed back: %s\n", buffer);
        }
    }

    // close socket
    close(socket_fd);
    return 0;
}