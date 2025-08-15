#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "aquarium.h"

#define BUFFER_SIZE 1024

void test_create_vue() {
    printf("%s", __func__);

    Vue v = create_vue("N1", 0, 0, 500, 500);
    
    assert(strcmp(v.nom, "N1") == 0);
    assert(v.x == 0 && v.y == 0);
    assert(v.width == 500 && v.height == 500);
    
    printf("\tOK\n");
}

void test_add_vue() {
    printf("%s", __func__);
    
    Aquarium aqua = {1000, 1000, {}, 0};
    Vue v = create_vue("N1", 0, 0, 500, 500);
    
    assert(add_vue(&aqua, v) == 1);
    assert(aqua.nb_vues == 1);
    assert(strcmp(aqua.vues[0].nom, "N1") == 0);
    assert(aqua.vues[0].x == 0 && aqua.vues[0].y == 0);
    assert(aqua.vues[0].width == 500 && aqua.vues[0].height == 500);
    
    printf("\tOK\n");
}

void test_del_vue() {
    printf("%s", __func__);
    
    Aquarium aqua = {1000, 1000, {}, 0};
    add_vue(&aqua, create_vue("N1", 0, 0, 500, 500));
    
    assert(del_vue(&aqua, "N1") == 1);
    assert(aqua.nb_vues == 0);
    
    printf("\tOK\n");
}

// void test_format_aquarium() {
//     printf("%s", __func__);
//     
//     Aquarium aqua = {1000, 1000, {}, 0};
//     add_vue(&aqua, create_vue("N1", 0, 0, 500, 500));
//     add_vue(&aqua, create_vue("N2", 500, 0, 500, 500));
//     char buffer[BUFFER_SIZE];
//     format_aquarium(&aqua, buffer, BUFFER_SIZE);
//     
//     assert(strstr(buffer, "N1") != NULL);
//     assert(strstr(buffer, "N2") != NULL);
//     
//     printf("\tOK\n");
// }

void test_save_aquarium() {
    printf("%s", __func__);
    
    Aquarium aqua = {1000, 1000, {}, 0};
    add_vue(&aqua, create_vue("N1", 0, 0, 500, 500));
    add_vue(&aqua, create_vue("N2", 500, 0, 500, 500));
    
    assert(save_aquarium(&aqua, "test_aqua.txt") == 1);
    
    printf("\tOK\n");
}

void test_load_aquarium() {
    printf("%s", __func__);
    
    Aquarium loaded;
    
    assert(load_aquarium(&loaded, "test_aqua.txt") == 1);
    assert(loaded.nb_vues == 2);
    assert(strcmp(loaded.vues[0].nom, "N1") == 0);
    assert(strcmp(loaded.vues[1].nom, "N2") == 0);
    
    printf("\tOK\n");
}

void test_aquarium() {
    printf("%s\n", __FILE__);
    
    test_create_vue();
    test_add_vue();
    test_del_vue();
    // test_format_aquarium();
    test_save_aquarium();
    test_load_aquarium();
}
