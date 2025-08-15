#ifndef POISSONS_H
#define POISSONS_H

#include "aquarium.h"

#define MAX_NAME 20
#define MAX_POISSONS 100

// Structure représentant un poisson
typedef struct {
    char name[MAX_NAME];         // Nom du poisson
    float width, height;         // Taille du poisson (en pourcentage de l'aquarium)
    char mobilityModel[MAX_NAME]; // Modèle de mobilité (RandomWayPoint, etc.)
    int started;                 // Indique si le poisson est en mouvement (1 = oui, 0 = non)
    int start_x;                 // Position de départ du poisson
    int start_y;
    int target_x;                // Position cible du poisson
    int target_y;
    int remaining_duration;      // Temps restant avant d'atteindre la cible
    int waiting_to_join;         // Indique si le poisson attend de rejoindre le banc
} Fish;

// Structure représentant un banc de poissons
typedef struct {
    Fish fishes[MAX_POISSONS];   // Liste des poissons
    int nb_poissons;             // Nombre actuel de poissons
    int common_duration;         // Durée commune pour tous les poissons
    int is_moving;               // Indique si le banc est en mouvement
} ShoalFish;

// Ajoute un poisson à l'aquarium
int add_fish(ShoalFish *shoal, const char *name, float x, float y, float width, float height, const char *mobilityModel);

// Supprime un poisson de l'aquarium
int del_fish(ShoalFish *shoal, const char *name);

// Démarre le mouvement d'un poisson
int start_fish(ShoalFish *shoal, const char *name);

// Met à jour la position d'un poisson selon son modèle de mobilité
void update_fish_position(ShoalFish *shoal, int elapsedTime);

// Affiche l'état actuel des poissons
void show_fishes(const ShoalFish *shoal);

#endif // POISSONS_H
