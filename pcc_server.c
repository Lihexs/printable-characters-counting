#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <errno.h>

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10
#define NUM_PRINTABLE_CHARS 95

unsigned int pcc_total[NUM_PRINTABLE_CHARS] = {0};
int server_socket;
volatile sig_atomic_t running = 1;
volatile sig_atomic_t handling_client = 0;

void handle_sigint(int sig);
void handle_client(int client_socket);
void print_statistics();

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("socket creation failed");
        exit(1);
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[1]));
    addr.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        perror("setsockopt failed");
        exit(1);
    }

    if (bind(server_socket, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("bind failed");
        exit(1);
    }

    if (listen(server_socket, MAX_CLIENTS) == -1)
    {
        perror("listen failed");
        exit(1);
    }

    while (running)
    {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);

        if (client_socket == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else
            {
                perror("accept failed");
                continue;
            }
        }

        handling_client = 1;
        handle_client(client_socket);
        handling_client = 0;

        if (!running)
            break;
    }

    close(server_socket);
    print_statistics();
    return 0;
}

void handle_sigint(int sig)
{
    running = 0;
    if (!handling_client)
    {
        close(server_socket);
        print_statistics();
        exit(0);
    }
}

void handle_client(int client_socket)
{
    uint32_t file_size;
    ssize_t bytes_read = recv(client_socket, &file_size, sizeof(file_size), 0);
    if (bytes_read != sizeof(file_size))
    {
        perror("Failed to receive file size");
        close(client_socket);
        return;
    }
    file_size = ntohl(file_size);

    char buffer[BUFFER_SIZE];
    uint32_t printable_count = 0;
    uint32_t total_read = 0;
    unsigned int temp_pcc[NUM_PRINTABLE_CHARS] = {0};

    while (total_read < file_size)
    {
        bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_read <= 0)
        {
            if (bytes_read == 0 || errno == ETIMEDOUT || errno == ECONNRESET || errno == EPIPE)
            {
                fprintf(stderr, "Client connection terminated unexpectedly\n");
                close(client_socket);
                return;
            }
            perror("recv failed");
            close(client_socket);
            return;
        }

        for (int i = 0; i < bytes_read; i++)
        {
            if (buffer[i] >= 32 && buffer[i] <= 126)
            {
                printable_count++;
                temp_pcc[buffer[i] - 32]++;
            }
        }
        total_read += bytes_read;
    }

    uint32_t count_to_send = htonl(printable_count);
    if (send(client_socket, &count_to_send, sizeof(count_to_send), 0) == -1)
    {
        perror("Failed to send count");
        close(client_socket);
        return;
    }

    for (int i = 0; i < NUM_PRINTABLE_CHARS; i++)
    {
        pcc_total[i] += temp_pcc[i];
    }

    close(client_socket);
}

void print_statistics()
{
    for (int i = 0; i < NUM_PRINTABLE_CHARS; i++)
    {
        printf("char '%c' : %u times\n", i + 32, pcc_total[i]);
    }
}