#ifndef SERVER_CONTEXT_H
#define SERVER_CONTEXT_H

#include <sys/select.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include "aquarium.h"
#include <stdbool.h>

#define MAX_VUES 40
#define MAX_BUFFER_SIZE 2048

typedef struct {
    int socket_fd;
    int vue_index;
    time_t last_activity;
    bool wants_continuous_updates;
    time_t last_update_time;
    bool should_disconnect;  // Nouveau champ pour marquer les clients à déconnecter
} ClientInfo;

typedef struct {
    fd_set active_fds;
    int max_fd;
    struct timeval timeout;
    int server_socket;
    ClientInfo clients[MAX_VUES]; 
    char client_buffers[MAX_VUES][MAX_BUFFER_SIZE];
} ServerContext;

int create_server_socket(int port);
ServerContext init_server_context(int serveur_sock_fd);

void accept_client_connection(int server_sock_fd, ServerContext *serveur_context);
void disconnect_client(int client_fd, ServerContext* ctx, Aquarium* aqua);
int receive_client_message(int sockfd, char *buffer, size_t buffer_size);
int send_client_message(int sockfd, const char *message);

#endif // SERVER_CONTEXT_H