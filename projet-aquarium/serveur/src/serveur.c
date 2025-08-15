#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>

#include "aquarium.h"
#include "log.h"
#include "config.h"
#include "server_context.h"
#include "fish_movement.h"

#define BUFLEN 256

ShoalFish shoal = { .nb_poissons = 0 };

void process_command(char *command, Aquarium *aqua) {
    char arg1[BUFLEN];
    int vue_x, vue_y, vue_width, vue_height;

    if (strncmp(command, "load", 4) == 0) {
        sscanf(command, "load %s", arg1);
        load_aquarium(aqua, arg1);
        printf("\t-> aquarium loaded (%d display view)!\n", aqua->nb_vues);

    } else if (strncmp(command, "show aquarium", 13) == 0) {
        show_aquarium(aqua);

    } else if (strncmp(command, "add view", 8) == 0) {
        sscanf(command, "add view %s %dx%d+%d+%d", arg1, &vue_x, &vue_y, &vue_width, &vue_height);
        Vue new_view = create_vue(arg1, vue_x, vue_y, vue_width, vue_height);
        add_vue(aqua, new_view);
        printf("\t-> view added\n");

    } else if (strncmp(command, "del view", 8) == 0) {
        sscanf(command, "del view %s", arg1);
        del_vue(aqua, arg1);
        printf("\t-> view %s deleted\n", arg1);

    } else if (strncmp(command, "save", 4) == 0) {
        sscanf(command, "save %s", arg1);
        save_aquarium(aqua, arg1);
        printf("\t-> aquarium saved! (%d display view)\n", aqua->nb_vues);

    } else {
        log_info("Commande inconnue.");
    }
}

void process_greeting(char *received_id, char *response_message, Aquarium *aqua, int client_fd, ServerContext *ctx) {
    Vue *vue_ptr;
    int client_slot = -1;
    
    // Trouver le slot du client
    for (int i = 0; i < MAX_VUES; i++) {
        if (ctx->clients[i].socket_fd == client_fd) {
            client_slot = i;
            break;
        }
    }
    
    if (client_slot == -1) {
        strcpy(response_message, "no greeting\n");
        return;
    }
    
    if (get_vue_by_id(aqua, received_id, &vue_ptr) && vue_ptr->owner_fd == -1) {
        // Vue trouvée et non occupée
        vue_ptr->owner_fd = client_fd;
        ctx->clients[client_slot].vue_index = vue_ptr - aqua->vues; // Obtenir l'index
        ctx->clients[client_slot].last_activity = time(NULL);
        
        snprintf(response_message, BUFLEN, "greeting %s %dx%d+%d+%d %dx%d\n", 
                received_id, vue_ptr->x, vue_ptr->y, vue_ptr->width, vue_ptr->height,
                aqua->width, aqua->height);
    } else {
        // Vue déjà prise ou n'existe pas, proposer une alternative
        if (get_free_vue(aqua, &vue_ptr)) {
            vue_ptr->owner_fd = client_fd;
            ctx->clients[client_slot].vue_index = vue_ptr - aqua->vues; // Obtenir l'index
            ctx->clients[client_slot].last_activity = time(NULL);
            
            snprintf(response_message, BUFLEN, "greeting %s %dx%d+%d+%d %dx%d\n", 
                    vue_ptr->nom, vue_ptr->x, vue_ptr->y, 
                    vue_ptr->width, vue_ptr->height,
                    aqua->width, aqua->height);
        } else {
            strcpy(response_message, "no greeting\n");
            return;
        }
    }

    log_info("Client %d prend la vue %s", client_fd, vue_ptr->nom);
}



void process_client_message(char *received_message, char *response_message, Aquarium *aqua, int client_fd, ServerContext *ctx) {
    // Nettoyage des caractères non imprimables
    char cleaned_message[BUFLEN];
    response_message[0] = '\0';

    // Ignorer les caractères non imprimables en début de message
    while (*received_message && (*received_message < 32 || *received_message > 126)) {
        received_message++;
    }

    // Copier le reste dans un buffer propre
    strncpy(cleaned_message, received_message, BUFLEN - 1);
    cleaned_message[BUFLEN - 1] = '\0';

    log_info("Process message du client %d: %s", client_fd, cleaned_message);

    char arg1[BUFLEN - 25];
    Vue *vue_ptr;
    int client_slot = -1;
    
    // Trouver le slot du client
    for (int i = 0; i < MAX_VUES; i++) {
        if (ctx->clients[i].socket_fd == client_fd) {
            client_slot = i;
            break;
        }
    }
    
    if (client_slot == -1) {
        log_warn("Client %d non trouvé dans la table des clients", client_fd);
        return;
    }
    
    // Mise à jour de l'activité du client
    ctx->clients[client_slot].last_activity = time(NULL);

    if (strncmp(cleaned_message, "hello in as", 11) == 0) {
        sscanf(cleaned_message, "hello in as %s", arg1);
        process_greeting(arg1, response_message, aqua, client_fd, ctx);

    } else if (strncmp(cleaned_message, "hello", 5) == 0) {
        process_greeting("", response_message, aqua, client_fd, ctx);
    
    }else if (strncmp(cleaned_message, "getFishesContinuously", strlen("getFishesContinuously")) == 0) {
        // Activer les mises à jour continues pour ce client
        ctx->clients[client_slot].wants_continuous_updates = true;
        ctx->clients[client_slot].last_update_time = time(NULL);
        log_info("Mises à jour continues activées pour le client %d", client_fd);
        
        // Envoyer l'état initial des poissons
        send_fish_list_step(response_message, BUFLEN, aqua, &shoal, BUFLEN);
        if (write(client_fd, response_message, strlen(response_message)) < 0) {
            perror("Erreur lors de l'envoi de l'état initial");
            disconnect_client(client_fd, ctx, aqua);
            return;
        }

    } else if (strncmp(cleaned_message, "getFishes", 9) == 0) {
        generate_fish_list(response_message, aqua, &shoal, BUFLEN);

    } else if (strncmp(cleaned_message, "ls", 2) == 0) {
        int num_steps = 3;
        int temp;

        if (sscanf(cleaned_message, "ls %d", &temp) == 1 && temp > 0 && temp <= 10) {
            num_steps = temp;
        }

        for (int step = 0; step < num_steps; step++) {
            send_fish_list_step(response_message, BUFLEN, aqua, &shoal, BUFLEN);
        }

    } else if (strncmp(cleaned_message, "log out", 7) == 0) {
        // Envoyer la réponse bye
        snprintf(response_message, BUFLEN, "bye\n");
        if (write(client_fd, response_message, strlen(response_message)) < 0) {
            log_error("Erreur lors de l'envoi de la réponse bye");
        }
        // Marquer le client pour déconnexion dans la prochaine itération
        for (int i = 0; i < MAX_VUES; i++) {
            if (ctx->clients[i].socket_fd == client_fd) {
                ctx->clients[i].should_disconnect = true;
                break;
            }
        }
        return;

    } else if (strncmp(cleaned_message, "ping", 4) == 0) {
        sscanf(cleaned_message, "ping %s", arg1);
        snprintf(response_message, BUFLEN, "pong %s\n", arg1);

    } else if (strncmp(cleaned_message, "addFish", 7) == 0) {
        char fish_name[32], mobility[32];
        float fx, fy, fwidth, fheight;

        int matched = sscanf(cleaned_message, "addFish %31s at %f%*c%f, %f%*c%f, %31s",
                            fish_name, &fx, &fy, &fwidth, &fheight, mobility);

        if (matched != 6) {
            snprintf(response_message, BUFLEN, "NOK\n");
            log_warn("Format incorrect pour la commande addFish : \"%s\"", cleaned_message);
        } else if (!add_fish(&shoal, fish_name, fx, fy, fwidth, fheight, mobility)) {
            snprintf(response_message, BUFLEN, "NOK\n");
        } else {
            snprintf(response_message, BUFLEN, "OK\n");
            log_info("Poisson ajouté : %s à %.2fx%.2f, taille %.2fx%.2f, modèle %s",
                    fish_name, fx, fy, fwidth, fheight, mobility);
        }

    } else if (strncmp(cleaned_message, "delFish", 7) == 0) {
        sscanf(cleaned_message, "delFish %s", arg1);
        if (!del_fish(&shoal, arg1)) {
            snprintf(response_message, BUFLEN, "NOK\n");
        } else {
            snprintf(response_message, BUFLEN, "OK\n");
        }

    } else if (strncmp(cleaned_message, "startFish", 9) == 0) {
        sscanf(cleaned_message, "startFish %s", arg1);
        if (!start_fish(&shoal, arg1)) {
            snprintf(response_message, BUFLEN, "NOK\n");
        } else {
            snprintf(response_message, BUFLEN, "OK\n");
            // Envoyer immédiatement les mises à jour à tous les clients
            char update_message[BUFLEN];
            send_fish_list_step(update_message, BUFLEN, aqua, &shoal, BUFLEN);
            if (strlen(update_message) > 0) {
                for (int i = 0; i < MAX_VUES; i++) {
                    if (ctx->clients[i].socket_fd != -1 && 
                        ctx->clients[i].wants_continuous_updates) {
                        if (write(ctx->clients[i].socket_fd, update_message, strlen(update_message)) < 0) {
                            perror("Erreur lors de l'envoi des mises à jour continues");
                            disconnect_client(ctx->clients[i].socket_fd, ctx, aqua);
                        }
                        log_info("Envoi des positions des poissons au client %d :\n%s", 
                                ctx->clients[i].socket_fd, update_message);
                    }
                }
            }
        }

    } else {
        log_info("Commande inconnue reçue.");
    }

    log_info("Réponse au client %d: %s", client_fd, response_message);
}


void handle_received_data(ServerContext* ctx, int client_fd, Aquarium* aqua) {
    char buffer[BUFLEN];
    int len = receive_client_message(client_fd, buffer, BUFLEN);

    if (len <= 0) {
        // Déconnexion ou erreur
        log_warn("Erreur de réception pour le client %d, déconnexion", client_fd);
        disconnect_client(client_fd, ctx, aqua);
        return;
    }

    // Trouver l'index du client dans la table
    int client_index = -1;
    for (int i = 0; i < MAX_VUES; i++) {
        if (ctx->clients[i].socket_fd == client_fd) {
            client_index = i;
            break;
        }
    }
    
    if (client_index == -1) {
        log_warn("Client %d non trouvé dans la table des clients", client_fd);
        return;
    }

    // Mise à jour du temps d'activité
    ctx->clients[client_index].last_activity = time(NULL);

    // Traitement des données reçues
    strncat(ctx->client_buffers[client_index], buffer, MAX_BUFFER_SIZE - strlen(ctx->client_buffers[client_index]) - 1);

    char *msg_start = ctx->client_buffers[client_index];
    char *newline = NULL;

    // Tant qu'il y a des messages complets
    while ((newline = strchr(msg_start, '\n')) != NULL) {
        *newline = '\0';  // Coupe ici

        // Copie dans un buffer propre pour traitement
        char clean_line[BUFLEN];
        strncpy(clean_line, msg_start, BUFLEN - 1);
        clean_line[BUFLEN - 1] = '\0';

        // Nettoyage des caractères non imprimables
        char *clean_ptr = clean_line;
        while (*clean_ptr && (*clean_ptr < 32 || *clean_ptr > 126)) {
            clean_ptr++;
        }

        char response[BUFLEN];
        response[0] = '\0';
        process_client_message(clean_ptr, response, aqua, client_fd, ctx);

        if (strlen(response) > 0) {
            if (write(client_fd, response, strlen(response)) < 0) {
                log_error("Erreur lors de l'envoi de la réponse au client %d", client_fd);
                disconnect_client(client_fd, ctx, aqua);
                return;
            }
        }

        // Décaler le buffer (enlever le message traité)
        msg_start = newline + 1;
    }

    // Copier ce qui reste (incomplet) au début du buffer
    memmove(ctx->client_buffers[client_index], msg_start, strlen(msg_start) + 1);
}



int main() {
    // Init serveur
    FILE *logfile = init_log();
    load_config();
    int server_sock_fd = create_server_socket(get_controller_port());
    ServerContext serveur_context = init_server_context(server_sock_fd);
    Aquarium aqua;

    log_info("Serveur démarré !");
    
    // Initialiser la graine pour les nombres aléatoires
    srand(time(NULL));

    // Variables pour le suivi du temps
    time_t last_fish_update = time(NULL);
    time_t current_time;

    while (1) {
        fd_set read_fds = serveur_context.active_fds;
        current_time = time(NULL);

        if (select(serveur_context.max_fd + 1, &read_fds, NULL, NULL, &(serveur_context.timeout)) < 0) {
            log_error("Erreur lors du select()");
            break;
        }

        if (FD_ISSET(server_sock_fd, &read_fds)) {
            accept_client_connection(server_sock_fd, &serveur_context);
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            char command[BUFLEN];
            if (fgets(command, BUFLEN, stdin) != NULL) {
                command[strcspn(command, "\n")] = 0;
                process_command(command, &aqua);
            }
        }

        // Calculer le temps réel écoulé depuis la dernière mise à jour
        int elapsed_seconds = (int)difftime(current_time, last_fish_update);
        if (elapsed_seconds > 0) {
            // Mettre à jour les durées restantes pour tous les poissons
            update_fish_durations(&shoal, elapsed_seconds);
            last_fish_update = current_time;

            // Vérifier si au moins un poisson a atteint sa destination ou vient d'être démarré
            bool any_fish_reached_destination = false;
            for (int i = 0; i < shoal.nb_poissons; i++) {
                if (shoal.fishes[i].started && 
                    (shoal.fishes[i].remaining_duration == 0 || 
                     shoal.fishes[i].start_x == shoal.fishes[i].target_x)) {
                    any_fish_reached_destination = true;
                    break;
                }
            }

            // Si un poisson a atteint sa destination ou vient d'être démarré, envoyer les mises à jour à tous les clients
            if (any_fish_reached_destination) {
                char update_message[BUFLEN];
                send_fish_list_step(update_message, BUFLEN, &aqua, &shoal, BUFLEN);
                
                // Ne pas envoyer si le message est vide (aucun poisson à mettre à jour)
                if (strlen(update_message) > 0) {
                    // Envoyer à tous les clients qui ont activé les mises à jour continues
                    for (int i = 0; i < MAX_VUES; i++) {
                        if (serveur_context.clients[i].socket_fd != -1 && 
                            serveur_context.clients[i].wants_continuous_updates) {
                            if (write(serveur_context.clients[i].socket_fd, update_message, strlen(update_message)) < 0) {
                                perror("Erreur lors de l'envoi des mises à jour continues");
                                disconnect_client(serveur_context.clients[i].socket_fd, &serveur_context, &aqua);
                            }
                            log_info("Envoi des positions des poissons au client %d :\n%s", 
                                    serveur_context.clients[i].socket_fd, update_message);
                        }
                    }
                }
            }
        }

        // Traitement des messages clients
        for (int i = 0; i < MAX_VUES; i++) {
            int sockfd = serveur_context.clients[i].socket_fd;
            if (sockfd != -1 && FD_ISSET(sockfd, &read_fds)) {
                handle_received_data(&serveur_context, sockfd, &aqua);
            }
        }

        // Vérification des clients inactifs (timeout)
        for (int i = 0; i < MAX_VUES; i++) {
            int sockfd = serveur_context.clients[i].socket_fd;
            if (sockfd != -1) {
                // Vérifier si le client doit être déconnecté
                if (serveur_context.clients[i].should_disconnect) {
                    log_info("Déconnexion du client %d suite à une demande de log out", sockfd);
                    disconnect_client(sockfd, &serveur_context, &aqua);
                    continue;
                }
                // Vérifier le timeout
                if (difftime(current_time, serveur_context.clients[i].last_activity) > get_display_timeout_value()) {
                    log_info("Client %d inactif depuis trop longtemps, déconnexion", sockfd);
                    disconnect_client(sockfd, &serveur_context, &aqua);
                }
            }
        }
    }

    // Fermeture du serveur
    close(server_sock_fd);
    log_info("Arrêt du serveur.");
    fclose(logfile);
    return 0;
}
