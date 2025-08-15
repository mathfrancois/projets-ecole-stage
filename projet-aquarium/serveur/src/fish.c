#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fish.h"
#include "log.h"

// Ajoute un poisson à l'aquarium
int add_fish(ShoalFish *shoal, const char *name, float x, float y, float width, float height, const char *mobilityModel) {
    if (shoal->nb_poissons >= MAX_POISSONS) {
        log_error("Nombre maximal de poissons atteint (%d).", MAX_POISSONS);
        return 0;
    }
    
    // Vérifier si le poisson existe déjà
    for (int i = 0; i < shoal->nb_poissons; i++) {
        if (strcmp(shoal->fishes[i].name, name) == 0) {
            log_warn("Un poisson avec le nom '%s' existe déjà.", name);
            return 0;
        }
    }

    // Ajouter le poisson
    Fish *new_fish = &shoal->fishes[shoal->nb_poissons++];
    strncpy(new_fish->name, name, MAX_NAME);
    
    // Initialiser les positions
    new_fish->start_x = (int)x;
    new_fish->start_y = (int)y;
    new_fish->target_x = (int)x;
    new_fish->target_y = (int)y;
    
    new_fish->width = width;
    new_fish->height = height;
    strncpy(new_fish->mobilityModel, mobilityModel, MAX_NAME);

    // Initialisation du comportement
    new_fish->started = false;                  // Pas encore démarré
    new_fish->remaining_duration = 0;           // aucune durée restante au départ

    log_info("Poisson ajouté : %s à %.1fx%.1f, taille %.1fx%.1f, modèle %s.", name, x, y, width, height, mobilityModel);
    return 1;
}

// Supprime un poisson de l'aquarium
int del_fish(ShoalFish *shoal, const char *name) {
    for (int i = 0; i < shoal->nb_poissons; i++) {
        if (strcmp(shoal->fishes[i].name, name) == 0) {
            // Décaler les poissons restants pour remplir le trou
            for (int j = i; j < shoal->nb_poissons - 1; j++) {
                shoal->fishes[j] = shoal->fishes[j + 1];
            }
            shoal->nb_poissons--;
            log_info("Poisson supprimé : %s.", name);
            return 1;
        }
    }
    log_error("Impossible de supprimer le poisson '%s' : non trouvé.", name);
    return 0;
}

// Démarre le mouvement d'un poisson
int start_fish(ShoalFish *shoal, const char *name) {
    for (int i = 0; i < shoal->nb_poissons; i++) {
        if (strcmp(shoal->fishes[i].name, name) == 0) {
            Fish *fish = &shoal->fishes[i];
            
            if (shoal->is_moving) {
                // Si le banc est en mouvement, marquer le poisson comme en attente
                fish->started = 1;
                fish->waiting_to_join = 1;
                fish->target_x = fish->start_x;
                fish->target_y = fish->start_y;
                fish->remaining_duration = 0;
                log_info("Poisson '%s' en attente de rejoindre le banc", name);
            } else {
                // Si le banc n'est pas en mouvement, démarrer normalement
                fish->started = 1;
                fish->waiting_to_join = 0;
                fish->target_x = fish->start_x;
                fish->target_y = fish->start_y;
                fish->remaining_duration = 0;
                
                // Marquer le banc comme en mouvement
                shoal->is_moving = 1;
                // Définir une durée commune pour tous les poissons (entre 3 et 10 secondes)
                shoal->common_duration = 3 + rand() % 8;
                
                log_info("Poisson '%s' démarré avec une durée commune de %d secondes", name, shoal->common_duration);
            }
            return 1;
        }
    }
    log_error("Impossible de démarrer le poisson '%s' : non trouvé.", name);
    return 0;
}

// Met à jour la position des poissons selon leur modèle de mobilité
void update_fish_position(ShoalFish *shoal, int elapsedTime) {
    for (int i = 0; i < shoal->nb_poissons; i++) {
        Fish *fish = &shoal->fishes[i];
        if (fish->started) {
            if (strcmp(fish->mobilityModel, "RandomWayPoint") == 0) {
                // Déplacement aléatoire simple
                int old_x = fish->start_x;
                int old_y = fish->start_y;

                fish->start_x += (rand() % 21) - 10; // Variation entre -10 et +10
                fish->start_y += (rand() % 21) - 10;
                
                // Vérification des limites de l'aquarium
                if (fish->start_x < 0) fish->start_x = 0;
                if (fish->start_y < 0) fish->start_y = 0;
                if (fish->start_x > 100) fish->start_x = 100;
                if (fish->start_y > 100) fish->start_y = 100;

                log_debug("Déplacement de '%s' de (%d,%d) à (%d,%d).", 
                         fish->name, old_x, old_y, fish->start_x, fish->start_y);
            }
        }
    }
}

// Affiche l'état actuel des poissons
void show_fishes(const ShoalFish *shoal) {
    printf("Nombre de poissons : %d\n", shoal->nb_poissons);

    for (int i = 0; i < shoal->nb_poissons; i++) {
        Fish fish = shoal->fishes[i];
        printf("Fish %s at %dx%d (target: %dx%d), %.1fx%.1f, %s %s\n",
               fish.name, fish.start_x, fish.start_y, fish.target_x, fish.target_y,
               fish.width, fish.height, fish.mobilityModel,
               fish.started ? "started" : "notStarted");
    }
}
