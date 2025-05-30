#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <bits/getopt_core.h>

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

        buffer[bytes_read] = '\0';
        printf("Received: %s", buffer);

        if (strncmp(buffer, "QUIT", 4) == 0) {
            send(client_fd, "Goodbye!\n", 9, 0);
            break;
        } else if (strncmp(buffer, "USER", 4) == 0) {
            sscanf(buffer, "USER %s %s", username, password);
            if (authenticate_user(username, password, "passwords.cfg")) {
                send(client_fd, "200 User authenticated.\n", 25, 0);
                authenticated = 1;
            } else {
                send(client_fd, "400 Invalid credentials.\n", 25, 0);
            }
        } else if (!authenticated) {
            send(client_fd, "403 Please authenticate first.\n", 31, 0);
        } else if (strncmp(buffer, "LIST", 4) == 0) {
            DIR *dir = opendir(".");
            if (dir) {
                struct dirent *entry;
                while ((entry = readdir(dir)) != NULL) {
                    if (entry->d_type == DT_REG) {
                        snprintf(buffer, sizeof(buffer), "%s\n", entry->d_name);
                        send(client_fd, buffer, strlen(buffer), 0);
                    }
                }
                send(client_fd, ".\n", 2, 0);
                closedir(dir);
            }
  } else if (strncmp(buffer, "GET", 3) == 0) {
            char filename[100];
            sscanf(buffer, "GET %s", filename);
            FILE *file = fopen(filename, "rb");
            if (file) {
                while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
                    send(client_fd, buffer, bytes_read, 0);
                }
                fclose(file);
                send(client_fd, "\r\n.\r\n", 5, 0);
            } else {
                send(client_fd, "404 File not found.\n", 20, 0);
            }
        } else if (strncmp(buffer, "PUT", 3) == 0) {
            char filename[100];
            sscanf(buffer, "PUT %s", filename);
            FILE *file = fopen(filename, "wb");
            if (file) {
                while ((bytes_read = recv(client_fd, buffer, sizeof(buffer), 0)) > 0) {
                    if (strstr(buffer, "\r\n.\r\n")) {
                        fwrite(buffer, 1, bytes_read - 5, file);
                        break;
                    }
                    fwrite(buffer, 1, bytes_read, file);
                }
                fclose(file);
                send(client_fd, "200 File saved.\n", 17, 0);
            } else {
                send(client_fd, "400 File cannot save.\n", 23, 0);
            }
        } else if (strncmp(buffer, "DEL", 3) == 0) {
            char filename[100];
            sscanf(buffer, "DEL %s", filename);
            if (remove(filename) == 0) {
                send(client_fd, "200 File deleted.\n", 19, 0);
            } else {
                send(client_fd, "404 File not found.\n", 20, 0);
            }
        } else {
            send(client_fd, "500 Unknown command.\n", 22, 0);
        }
    }

    close(client_fd);
    return NULL;
}

/* User Authentication */
int authenticate_user(const char *username, const char *password, const char *password_file) {
    FILE *file = fopen(password_file, "r");
    if (!file) return 0;

    char line[100];
    while (fgets(line, sizeof(line), file)) {
        char stored_user[50], stored_pass[50];
        sscanf(line, "%[^:]:%s", stored_user, stored_pass);
        if (strcmp(username, stored_user) == 0 && strcmp(password, stored_pass) == 0) {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

