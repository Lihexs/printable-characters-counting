#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

#define BUFFER_SIZE 4096

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <server_ip> <server_port> <file_path>\n", argv[0]);
        exit(1);
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &addr.sin_addr) <= 0)
    {
        perror("Invalid address");
        exit(1);
    }

    const char *file_path = argv[3];
    int fd = open(file_path, O_RDONLY);
    if (fd == -1)
    {
        perror("Failed to open file");
        exit(1);
    }

    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1)
    {
        perror("Socket creation failed");
        close(fd);
        exit(1);
    }

    if (connect(s, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("Connection failed");
        close(fd);
        close(s);
        exit(1);
    }

    off_t size_file_to_send = htonl((uint32_t)lseek(fd, 0, SEEK_END));
    lseek(fd, 0, SEEK_SET);
    if (send(s, &size_file_to_send, sizeof(size_file_to_send), 0) == -1)
    {
        perror("Failed to send file size");
        close(fd);
        close(s);
        exit(1);
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;
    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0)
    {
        if (send(s, buffer, bytesRead, 0) == -1)
        {
            perror("Failed to send file content");
            close(fd);
            close(s);
            exit(1);
        }
    }
    uint32_t count = 0;
    if (recv(s, &count, sizeof(count), 0) == -1)
    {
        perror("Failed to receive count");
        close(fd);
        close(s);
        exit(1);
    }

    printf("# of printable characters: %u\n", ntohl(count));

    close(fd);
    close(s);
    return 0;
}