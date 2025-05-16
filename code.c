#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>

/* Definitions */
#define DEFAULT_BUFLEN 1024
#define PORT 9634

void PANIC(char *msg);
#define PANIC(msg) { perror(msg); exit(EXIT_FAILURE); }

void *Child(void *arg);
int authenticate_user(const char *username, const char *password, const char *password_file);

/* Main Server Function */
int main(int argc, char *argv[]) {
    int sd, opt, optval = 1;
    struct sockaddr_in addr;
    unsigned short port = PORT;
    char *directory = NULL, *password_file = NULL;

    // Parse command-line arguments
    while ((opt = getopt(argc, argv, "d:p:u:")) != -1) {
        switch (opt) {
        case 'd':
            directory = optarg;
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case 'u':
            password_file = optarg;
            break;
        default:
            fprintf(stderr, "Usage: %s -d <directory> -p <port> -u <password_file>\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (!directory || !password_file) {
        fprintf(stderr, "Error: Directory and password file must be specified.\n");
        exit(EXIT_FAILURE);
    }

    if (chdir(directory) != 0) {
        perror("Failed to set directory");
        exit(EXIT_FAILURE);
    }

    // Create socket
    if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        PANIC("Socket creation failed");
    }

    // Set socket options
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    // Configure server address
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket
    if (bind(sd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        PANIC("Bind failed");
    }

    // Start listening
    if (listen(sd, SOMAXCONN) != 0) {
        PANIC("Listen failed");
    }

    printf("Server ready on port %d\n", port);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addr_size = sizeof(client_addr);
        pthread_t child;

        int client = accept(sd, (struct sockaddr *)&client_addr, &addr_size);
        if (client < 0) {
            perror("Accept failed");
            continue;
        }

        printf("Connected: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        int *client_sock = malloc(sizeof(int));
        *client_sock = client;

        if (pthread_create(&child, NULL, Child, client_sock) != 0) {
            perror("Thread creation failed");
        } else {
            pthread_detach(child);
        }
    }

    close(sd);
    return 0;
}
