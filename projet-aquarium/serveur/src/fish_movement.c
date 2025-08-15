#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "fish_movement.h"
#include "aquarium.h"
#include "config.h"
#include <stdbool.h>
#include "log.h"

void generate_next_fish_step(Aquarium *aqua, Fish *fish, int *x, int *y, int *duration) {
    // Si poisson non démarré => rien à faire
    if (!fish->started) {
        *x = fish->start_x;
        *y = fish->start_y;
        *duration = 0;
        return;
    }

    // Cas où le poisson est encore en train de se déplacer vers sa cible
    if (fish->remaining_duration > 0) {
        *x = fish->target_x;
        *y = fish->target_y;
        *duration = fish->remaining_duration;
        return;
    }

    // Le poisson a atteint sa destination et la durée est écoulée
    // On met à jour sa position de départ AVANT de calculer la nouvelle destination
    fish->start_x = fish->target_x;
    fish->start_y = fish->target_y;

    // Calculer une nouvelle destination
    int new_x, new_y, new_dur;
    generate_random_waypoint(aqua, fish, &new_x, &new_y, &new_dur);

    // Ne mettre à jour que si la nouvelle position est différente
    if (new_x != fish->start_x || new_y != fish->start_y) {
        fish->target_x = new_x;
        fish->target_y = new_y;
        // Utiliser la durée commune du banc si elle existe
        fish->remaining_duration = fish->waiting_to_join ? 0 : new_dur;

        *x = new_x;
        *y = new_y;
        *duration = fish->remaining_duration;
    } else {
        // Si la position est la même, on ne change rien
        *x = fish->start_x;
        *y = fish->start_y;
        *duration = 0;
    }
}

void generate_random_waypoint(Aquarium* aqua, Fish *fish, int *x, int *y, int *duration) {
    // Générer une nouvelle position jusqu'à ce qu'elle soit différente de la position actuelle
    do {
        // Générer des coordonnées entre 0 et 100
        *x = rand() % 101; // Entre 0 et 100 inclus
        *y = rand() % 101;
    } while (*x == fish->start_x && *y == fish->start_y);

    // La durée est maintenant gérée au niveau du banc de poissons
    *duration = 0; // La durée sera définie par le banc
}

void send_fish_list_step(char *response_message, int response_size, Aquarium *aqua, ShoalFish *shoal, int bufflen) {
    response_message[0] = '\0';
    
    // Vérifier si au moins un poisson a atteint sa destination
    bool any_fish_to_update = false;
    for (int i = 0; i < shoal->nb_poissons; i++) {
        Fish *f = &shoal->fishes[i];
        if (f->started && f->remaining_duration == 0) {
            any_fish_to_update = true;
            break;
        }
    }

    // Si aucun poisson à mettre à jour, ne rien envoyer
    if (!any_fish_to_update) {
        return;
    }

    // Si au moins un poisson à mettre à jour, envoyer les listes
    strncat(response_message, "list ", response_size - strlen(response_message) - 1);

    // Première liste : positions de départ (uniquement pour les poissons qui ont atteint leur destination)
    for (int i = 0; i < shoal->nb_poissons; i++) {
        Fish *f = &shoal->fishes[i];
        
        // Ne traiter que les poissons démarrés qui ont atteint leur destination
        if (f->started && f->remaining_duration == 0) {
            // Mettre à jour la position de départ
            f->start_x = f->target_x;
            f->start_y = f->target_y;

            char fish_info[bufflen];
            snprintf(fish_info, sizeof(fish_info),
                     "[%s at %dx%d,%dx%d,0] ",
                     f->name,
                     f->start_x, f->start_y,
                     (int)f->width, (int)f->height);

            strncat(response_message, fish_info, response_size - strlen(response_message) - 1);
        }
    }

    strncat(response_message, "\nlist ", response_size - strlen(response_message) - 1);

    // Deuxième liste : positions d'arrivée (uniquement pour les poissons qui ont atteint leur destination)
    for (int i = 0; i < shoal->nb_poissons; i++) {
        Fish *f = &shoal->fishes[i];
        
        // Ne traiter que les poissons démarrés qui ont atteint leur destination
        if (f->started && f->remaining_duration == 0) {
            int abs_x = f->start_x;
            int abs_y = f->start_y;
            int duration = 0;

            // Calculer la nouvelle destination
            generate_next_fish_step(aqua, f, &abs_x, &abs_y, &duration);

            // Si le banc est en mouvement ou si c'est le premier mouvement
            if (shoal->is_moving || shoal->common_duration > 0) {
                duration = shoal->common_duration;
            } else {
                // Si c'est le premier mouvement, définir une nouvelle durée commune
                duration = 3 + rand() % 8; // Entre 3 et 10 secondes
                shoal->common_duration = duration;
                shoal->is_moving = 1;
            }
            
            // Mettre à jour la durée restante du poisson
            f->remaining_duration = duration;

            char fish_info[bufflen];
            snprintf(fish_info, sizeof(fish_info),
                     "[%s at %dx%d,%dx%d,%d] ",
                     f->name,
                     abs_x, abs_y,
                     (int)f->width, (int)f->height,
                     duration);

            strncat(response_message, fish_info, response_size - strlen(response_message) - 1);
        }
    }

    strncat(response_message, "\n", response_size - strlen(response_message) - 1);
}

void generate_fish_list(char *buffer, Aquarium *aqua, ShoalFish *shoal, int buferlen) {
    bzero(buffer, buferlen);
    strcat(buffer, "list ");

    for (int i = 0; i < shoal->nb_poissons; i++) {
        Fish *f = &shoal->fishes[i];

        int abs_x = f->start_x;
        int abs_y = f->start_y;
        int duration = f->remaining_duration;

        if (f->started) {
            generate_next_fish_step(aqua, f, &abs_x, &abs_y, &duration);
        }

        char fish_info[buferlen];
        snprintf(fish_info, sizeof(fish_info),
                 "[%s from %dx%d to %dx%d,%dx%d,%d] ",
                 f->name,
                 f->start_x, f->start_y,
                 abs_x, abs_y,
                 (int)f->width, (int)f->height,
                 duration);

        strcat(buffer, fish_info);
    }

    strcat(buffer, "\n");
}

void update_fish_durations(ShoalFish *shoal, int time_elapsed) {
    if (!shoal->is_moving) {
        return;
    }

    bool all_finished = true;
    for (int i = 0; i < shoal->nb_poissons; i++) {
        Fish *fish = &shoal->fishes[i];
        if (fish->started) {
            fish->remaining_duration -= time_elapsed;
            if (fish->remaining_duration < 0) {
                fish->remaining_duration = 0;
            }
            if (fish->remaining_duration > 0) {
                all_finished = false;
            }
        }
    }

    // Si tous les poissons ont terminé leur mouvement
    if (all_finished) {
        // Réinitialiser l'état du banc
        shoal->is_moving = 0;
        shoal->common_duration = 0;

        // Intégrer les poissons en attente
        for (int i = 0; i < shoal->nb_poissons; i++) {
            Fish *fish = &shoal->fishes[i];
            if (fish->waiting_to_join) {
                fish->waiting_to_join = 0;
                fish->remaining_duration = 0;
                log_info("Poisson '%s' intègre le banc", fish->name);
            }
        }
    }
}
