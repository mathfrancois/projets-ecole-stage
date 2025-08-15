#include "aquarium.h"

#include "log.h"

#include <stdio.h>
#include <string.h>

// Crée une Vue
Vue create_vue(const char *nom, int x, int y, int w, int h) {
    log_info("Création de la vue: %s, Position: (%d, %d), Taille: %dx%d", nom, x, y, w, h);
    
    Vue v;
    snprintf(v.nom, MAX_NOM, "%s", nom);
    v.owner_fd = -1;
    v.x = x;
    v.y = y;
    v.width = w;
    v.height = h;
    
    return v;
}

// Charge un aquarium depuis un fichier
int load_aquarium(Aquarium *aqua, const char *fichier) {
    log_info("Chargement de l'aquarium depuis le fichier: %s", fichier);
    
    FILE *fp = fopen(fichier, "r");
    if (!fp) {
        log_error("Erreur lors de l'ouverture du fichier '%s' en lecture.", fichier);
        return 0;
    }

    if (fscanf(fp, "%dx%d\n", &aqua->width, &aqua->height) != 2) {
        log_error("Erreur de lecture des dimensions de l'aquarium dans le fichier '%s'.", fichier);
        fclose(fp);
        return 0;
    }

    aqua->nb_vues = 0;
    while (aqua->nb_vues < MAX_VUES) {
        char nom[MAX_NOM];
        int x, y, width, height;
    
        if (fscanf(fp, "%s %dx%d+%d+%d\n", nom, &x, &y, &width, &height) != 5) {
            break;
        }

        Vue v = create_vue(nom, x, y, width, height);
        aqua->vues[aqua->nb_vues++] = v;
    }

    fclose(fp);
    return 1;
}

// Sauvegarde un aquarium dans un fichier
int save_aquarium(const Aquarium *aqua, const char *fichier) {
    log_info("Sauvegarde de l'aquarium dans le fichier: %s", fichier);

    FILE *fp = fopen(fichier, "w");
    if (!fp) {
        log_error("Erreur lors de l'ouverture du fichier '%s' en écriture.", fichier);
        return 0;
    }

    fprintf(fp, "%dx%d\n", aqua->width, aqua->height);

    for (int i = 0; i < aqua->nb_vues; i++) {
        fprintf(fp, "%s %dx%d+%d+%d\n",
                aqua->vues[i].nom, 
                aqua->vues[i].x, aqua->vues[i].y, 
                aqua->vues[i].width, aqua->vues[i].height);
    }

    fclose(fp);
    return 1;
}

// Ajoute une vue à l'aquarium
int add_vue(Aquarium *aqua, const Vue vue) {
    log_info("Ajout de la vue: %s", vue.nom);

    if (vue.x < 0 || vue.y < 0 
        || vue.x + vue.width > aqua->width 
        || vue.y + vue.height > aqua->height
        || aqua->nb_vues >= MAX_VUES) {
        log_error("La vue %s ne peut pas être ajoutée à l'aquarium (dimensions invalides ou trop de vues).", vue.nom);
        return 0;
    }

    aqua->vues[aqua->nb_vues++] = vue;
    return 1;
}

// Supprime une vue à l'aquarium
int del_vue(Aquarium *aqua, const char *nom) {
    log_info("Suppression de la vue: %s", nom);

    for (int i = 0; i < aqua->nb_vues; i++) {
        if (strcmp(aqua->vues[i].nom, nom) == 0) {
            for (int j = i; j < aqua->nb_vues - 1; j++) {
                aqua->vues[j] = aqua->vues[j + 1];
            }
            aqua->nb_vues--;
            return 1;
        }
    }
    log_error("Vue %s non trouvée dans l'aquarium.", nom);
    return 0;
}

// Affiche la topologie de l'aquarium
void show_aquarium(const Aquarium *aqua) {
    log_info("Génération de la topologie de l'aquarium.");
    
    printf("%dx%d\n", aqua->width, aqua->height);
    
    for (int i = 0; i < aqua->nb_vues; i++) {
        printf( "%s %dx%d+%d+%d\n",
                aqua->vues[i].nom,
                aqua->vues[i].x, 
                aqua->vues[i].y,
                aqua->vues[i].width,
                aqua->vues[i].height);
    }
}

// Récupère les information sur une vue
// return 0 si non present dans aquarium
int get_vue_by_id(const Aquarium *aqua, const char *nom, Vue **vue) {
    for (int i = 0; i < aqua->nb_vues; i++) {
        if (strcmp(aqua->vues[i].nom, nom) == 0) {
            *vue = (Vue *)&(aqua->vues[i]);
            return 1;
        }
    }
    return 0;
}

// Récupère les information sur une vue
// return 0 si non present dans aquarium
int get_vue_by_fd(const Aquarium *aqua, int fd, Vue **vue) {
    for (int i = 0; i < aqua->nb_vues; i++) {
        if (aqua->vues[i].owner_fd == fd) {
            *vue = (Vue *)&(aqua->vues[i]);
            return 1;
        }
    }
    return 0;
}

// Récupère une vue libre (owner == -1)
// return 0 si plus de vue libre
int get_free_vue(const Aquarium *aqua, Vue **vue) {
    for (int i = 0; i < aqua->nb_vues; i++) {
        if (aqua->vues[i].owner_fd == -1) {
           *vue = (Vue *)&(aqua->vues[i]);
            return 1; 
        }
    }
    return 0;
}
