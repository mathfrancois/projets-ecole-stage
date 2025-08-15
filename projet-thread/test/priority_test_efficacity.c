#include "thread_ext.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h>

void *task_priority() {
    for (int i = 0; i < 5; i++) {
        // Simule un travail
        usleep(10000); // 10 ms
        thread_yield_priority();
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("argument manquant: nombre de threads\n");
        return -1;
    }

    int nb_threads = atoi(argv[1]);
    thread_t *threads = malloc(sizeof(thread_t) * nb_threads);
    int *priorities = malloc(sizeof(int) * nb_threads);

    struct timeval tv1, tv2;
    unsigned long us;

    gettimeofday(&tv1, NULL);

    // Création des threads avec des priorités cycliques 0,1,2,...
    for (int i = 0; i < nb_threads; i++) {
        priorities[i] = i % 5; // Par exemple 0 à 4
        int err = thread_create_priority(&threads[i], task_priority, &priorities[i], priorities[i]);
        assert(!err);
    }

    // Join de tous les threads
    for (int i = 0; i < nb_threads; i++) {
        int err = thread_join_priority(threads[i], NULL);
        assert(!err);
    }

    gettimeofday(&tv2, NULL);
    us = (tv2.tv_sec - tv1.tv_sec) * 1000000 + (tv2.tv_usec - tv1.tv_usec);

    printf("%d threads exécutés avec scheduling par priorité en %lu us\n", nb_threads, us);
    printf("GRAPH;99;%d;%lu\n", nb_threads, us);  // 99 = ID du test (choisis-en un libre)

    free(threads);
    free(priorities);
    return 0;
}
