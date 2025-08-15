#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "fish.h"
#include "test_fish.h"

#define BUFFER_SIZE 1024

void test_add_fish() {
    printf("%s", __func__);

    ShoalFish shoal = { .nb_poissons = 0 };

    assert(add_fish(&shoal, "PoissonRouge", 50, 50, 10, 5, "RandomWayPoint") == 1);
    assert(shoal.nb_poissons == 1);
    assert(strcmp(shoal.fishes[0].name, "PoissonRouge") == 0);
    assert(shoal.fishes[0].x == 50 && shoal.fishes[0].y == 50);
    
    printf("\tOK\n");
}

void test_add_duplicate_fish() {
    printf("%s", __func__);

    ShoalFish shoal = { .nb_poissons = 0 };
    add_fish(&shoal, "PoissonRouge", 50, 50, 10, 5, "RandomWayPoint");

    assert(add_fish(&shoal, "PoissonRouge", 20, 30, 10, 5, "RandomWayPoint") == 0);
    assert(shoal.nb_poissons == 1);

    printf("\tOK\n");
}

void test_del_fish() {
    printf("%s", __func__);

    ShoalFish shoal = { .nb_poissons = 0 };
    add_fish(&shoal, "PoissonRouge", 50, 50, 10, 5, "RandomWayPoint");

    assert(del_fish(&shoal, "PoissonRouge") == 1);
    assert(shoal.nb_poissons == 0);

    printf("\tOK\n");
}

void test_del_non_existent_fish() {
    printf("%s", __func__);

    ShoalFish shoal = { .nb_poissons = 0 };
    add_fish(&shoal, "PoissonRouge", 50, 50, 10, 5, "RandomWayPoint");

    assert(del_fish(&shoal, "PoissonClown") == 0);
    assert(shoal.nb_poissons == 1);

    printf("\tOK\n");
}

void test_start_fish() {
    printf("%s", __func__);

    ShoalFish shoal = { .nb_poissons = 0 };
    add_fish(&shoal, "PoissonRouge", 50, 50, 10, 5, "RandomWayPoint");

    assert(start_fish(&shoal, "PoissonRouge") == 1);
    assert(shoal.fishes[0].started == 1);

    printf("\tOK\n");
}

void test_start_non_existent_fish() {
    printf("%s", __func__);

    ShoalFish shoal = { .nb_poissons = 0 };

    assert(start_fish(&shoal, "PoissonClown") == 0);

    printf("\tOK\n");
}

void test_update_fish_position() {
    printf("%s", __func__);

    ShoalFish shoal = { .nb_poissons = 0 };
    add_fish(&shoal, "PoissonRouge", 50, 50, 10, 5, "RandomWayPoint");
    start_fish(&shoal, "PoissonRouge");

    float old_x = shoal.fishes[0].x;
    float old_y = shoal.fishes[0].y;

    update_fish_position(&shoal, 1);

    assert(shoal.fishes[0].x != old_x || shoal.fishes[0].y != old_y);

    printf("\tOK\n");
}


void test_show_fishes() {
    printf("%s\n", __func__);

    ShoalFish shoal = { .nb_poissons = 0 };
    add_fish(&shoal, "PoissonRouge", 50, 50, 10, 5, "RandomWayPoint");
    add_fish(&shoal, "PoissonClown", 30, 30, 8, 4, "RandomWayPoint");

    show_fishes(&shoal);

    assert(shoal.nb_poissons == 2); // Vérification du nombre de poissons
    assert(strcmp(shoal.fishes[0].name, "PoissonRouge") == 0); // Vérification du premier poisson
    assert(strcmp(shoal.fishes[1].name, "PoissonClown") == 0); // Vérification du second poisson

    printf("\tOK\n");
}

void test_fish() {
    printf("%s\n", __FILE__);

    test_add_fish();
    test_add_duplicate_fish();
    test_del_fish();
    test_del_non_existent_fish();
    test_start_fish();
    test_start_non_existent_fish();
    test_update_fish_position();
    test_show_fishes();
}
