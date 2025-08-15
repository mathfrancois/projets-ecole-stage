#ifndef AQUARIUM_H
#define AQUARIUM_H

#define MAX_VUES 40
#define MAX_NOM 20

#include <time.h>

typedef struct {
    char nom[MAX_NOM];
    int owner_fd;   // -1 si pas de propriétaire
    time_t last_activity;
    int x, y;
    int width, height;
} Vue;

typedef struct {
    int width, height;
    Vue vues[MAX_VUES];
    int nb_vues;
} Aquarium;

// Crée une Vue
Vue create_vue(const char *nom, int x, int y, int w, int h);

// Charge un aquarium depuis un fichier
int load_aquarium(Aquarium *aqua, const char *fichier);

// Sauvegarde un aquarium dans un fichier
int save_aquarium(const Aquarium *aqua, const char *fichier);

// Ajoute une vue à l'aquarium
int add_vue(Aquarium *aqua, Vue vue);

// Supprime une vue à l'aquarium
int del_vue(Aquarium *aqua, const char *nom);

// Affiche la topologie de l'aquarium
void show_aquarium(const Aquarium *aqua);

// Récupère les information sur une vue
// return 0 si non present dans aquarium
int get_vue_by_id(const Aquarium *aqua, const char *nom, Vue **vue);

// Récupère les information sur une vue
// return 0 si non present dans aquarium
int get_vue_by_fd(const Aquarium *aqua, int fd, Vue **vue);

// Récupère une vue libre (owner == -1)
// return 0 si plus de vue libre
int get_free_vue(const Aquarium *aqua, Vue **vue);

#endif // AQUARIUM_H