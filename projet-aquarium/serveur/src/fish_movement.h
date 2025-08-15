#ifndef FISH_MOVEMENT_H
#define FISH_MOVEMENT_H

#include "fish.h"

//Génère le chemin que va suivre le poisson en fonction de sa mobilité (RandomPathWay ...)
void generate_next_fish_step(Aquarium *aqua, Fish *fish, int *x, int *y, int *duration);


//Génère un point aléatoire dans l'aquarium
void generate_random_waypoint(Aquarium* aqua, Fish *fish, int *x, int *y, int *duration);

//Génère la list à renvoyer au client quand ls est fait 
void send_fish_list_step(char *response_message, int response_size, Aquarium *aqua, ShoalFish *shoal, int bufflen);

void generate_fish_list(char *buffer, Aquarium *aqua, ShoalFish *shoal, int buferlen);

void update_fish_durations(ShoalFish *shoal, int time_elapsed);

void generate_horizontal_pathway(Aquarium *aqua, Fish *fish, int *x, int *y, int *duration);
#endif 