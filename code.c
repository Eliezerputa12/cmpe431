/* Client Handler */
void *Child(void *arg) {
    int client_fd = *(int *)arg;
    free(arg);

    char buffer[DEFAULT_BUFLEN];
    send(client_fd, "Welcome to the FTP server.\n", 27, 0);

    char username[50], password[50];
    int authenticated = 0;

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_read <= 0) {
            printf("Client disconnected.\n");
            break;
        }
