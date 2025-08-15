#include "server_context.h"

#include "log.h"

int create_server_socket(int port) {
    // Création de la socket (AF_INET : Domaine IPV4, SOCK_STREAM : Socket TCP)
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        log_error("Erreur lors de la création du socket");
        exit(EXIT_FAILURE);
    }

    // Option pour réutiliser le port rapidement
    int optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        log_error("Erreur lors du setsockopt()");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Déclaration et init de l'adresse du serveur
    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr)); // mise à zéro de serv_addr
    serv_addr.sin_family = AF_INET;  // utilisation d'IPV4

    // Définition de l'adresse IP
    int inet_status = inet_pton(AF_INET, "0.0.0.0", &serv_addr.sin_addr.s_addr); // convertit s_addr au format réseau (binaire)
    if (inet_status <= 0) {
        log_error("Adresse invalide ou non supportée");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Définition du port dans l'adresse du serveur
    serv_addr.sin_port = htons(port); // Convertir le port en format réseau 

    // Associe le socket à l'adresse et au port du serveur
    int bind_status = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if (bind_status < 0) {
        log_error("ERROR on bind()");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Met le socket en écoute pour les connexions entrantes
    int listen_status = listen(sockfd, MAX_VUES);
    if (listen_status < 0) {
        log_error("ERROR on listen()");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

ServerContext init_server_context(int server_socket) {
    ServerContext ctx;
    ctx.server_socket = server_socket;
    ctx.max_fd = server_socket;
    FD_ZERO(&ctx.active_fds);
    FD_SET(server_socket, &ctx.active_fds);
    FD_SET(STDIN_FILENO, &ctx.active_fds);
    ctx.timeout.tv_sec = 1;
    ctx.timeout.tv_usec = 0;

    for (int i = 0; i < MAX_VUES; i++) {
        ctx.clients[i].socket_fd = -1;
        ctx.clients[i].vue_index = -1;
        ctx.clients[i].last_activity = time(NULL);
        ctx.clients[i].wants_continuous_updates = false;
        ctx.clients[i].last_update_time = 0;
        ctx.clients[i].should_disconnect = false;
    }

    return ctx;
}


void accept_client_connection(int server_socket, ServerContext* ctx) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
    
    if (client_fd < 0) {
        log_error("Erreur lors de l'acceptation de la connexion client");
        return;
    }
    
    // Rechercher un slot libre pour ce client
    int slot = -1;
    for (int i = 0; i < MAX_VUES; i++) {
        if (ctx->clients[i].socket_fd == -1) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        log_warn("Nombre maximum de clients atteint, connexion refusée");
        close(client_fd);
        return;
    }
    
    // Initialiser le client
    ctx->clients[slot].socket_fd = client_fd;
    ctx->clients[slot].wants_continuous_updates = false;
    ctx->clients[slot].last_update_time = time(NULL);
    ctx->clients[slot].last_activity = time(NULL);
    ctx->clients[slot].vue_index = -1;
    ctx->client_buffers[slot][0] = '\0';
    
    // Mettre à jour le set de descripteurs
    FD_SET(client_fd, &ctx->active_fds);
    if (client_fd > ctx->max_fd) {
        ctx->max_fd = client_fd;
    }
    
    log_info("Nouvelle connexion client acceptée: %d", client_fd);
}


void disconnect_client(int client_fd, ServerContext* ctx, Aquarium* aqua) {
    // Trouver le client dans la structure
    int slot = -1;
    for (int i = 0; i < MAX_VUES; i++) {
        if (ctx->clients[i].socket_fd == client_fd) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        log_warn("Client %d non trouvé lors de la déconnexion", client_fd);
        return;
    }
    
    // Libérer la vue associée à ce client
    if (ctx->clients[slot].vue_index != -1) {
        aqua->vues[ctx->clients[slot].vue_index].owner_fd = -1;
        log_info("Vue %s libérée", aqua->vues[ctx->clients[slot].vue_index].nom);
    }
    
    // Nettoyer les ressources
    FD_CLR(client_fd, &ctx->active_fds);
    
    // Fermer le socket proprement
    shutdown(client_fd, SHUT_RDWR);  // Arrêter la lecture et l'écriture
    close(client_fd);
    
    // Réinitialiser complètement les informations du client
    memset(&ctx->clients[slot], 0, sizeof(ClientInfo));  // Mettre tout à zéro
    ctx->clients[slot].socket_fd = -1;
    ctx->clients[slot].vue_index = -1;
    ctx->clients[slot].last_activity = 0;
    ctx->clients[slot].wants_continuous_updates = false;
    ctx->clients[slot].last_update_time = 0;
    ctx->clients[slot].should_disconnect = false;
    
    // Réinitialiser le buffer du client
    memset(ctx->client_buffers[slot], 0, MAX_BUFFER_SIZE);
    
    log_info("Client %d déconnecté", client_fd);
}

int receive_client_message(int sockfd, char *buffer, size_t buffer_size) {
    bzero(buffer, buffer_size);
    int n = read(sockfd, buffer, buffer_size - 1);

    if (n <= 0) {
        close(sockfd);
        log_info("Client déconnecté.");
        return 0;
    }

    // Nettoyage du buffer : ne garder que caractères imprimables ou '\n'
    int j = 0;
    for (int i = 0; i < n; i++) {
        if ((buffer[i] >= 32 && buffer[i] <= 126) || buffer[i] == '\n') {
            buffer[j++] = buffer[i];
        }
    }
    buffer[j] = '\0';  // fin de chaîne propre

    return j;  // retourne le nombre d'octets utiles
}


int send_client_message(int sockfd, const char *message) {
    size_t total_sent = 0;
    size_t message_len = strlen(message);
    ssize_t sent;

    while (total_sent < message_len) {
        sent = write(sockfd, message + total_sent, message_len - total_sent);
        if (sent == -1) {
            perror("Erreur lors de l'envoi");
            return -1;
        }
        total_sent += sent;
    }
    return total_sent;  // Retourne le nombre total d'octets envoyés
}
