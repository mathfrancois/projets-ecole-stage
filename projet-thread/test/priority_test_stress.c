#include "thread_ext.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define HIGH_PRIORITY_THREAD_COUNT 100
#define LOW_PRIORITY_THREAD_COUNT 5
#define MAX_ITERATIONS 10

volatile int high_priority_done = 0;
volatile int low_priority_done = 0;

void *task_starvation(void *arg) {
    int priority = *(int *)arg;
    for (int i = 0; i < MAX_ITERATIONS; i++) {
        usleep(500); // 0.5ms
        thread_yield_priority();
    }

    if (priority == 0)
        __sync_fetch_and_add(&high_priority_done, 1);
    else if (priority == 4)
        __sync_fetch_and_add(&low_priority_done, 1);

    return NULL;
}

int main() {
    thread_t high_priority_threads[HIGH_PRIORITY_THREAD_COUNT];
    thread_t low_priority_threads[LOW_PRIORITY_THREAD_COUNT];
    int p_high = 0;
    int p_low = 4;

    printf("\n=== Stress Starvation Test ===\n");

    // Créer les threads de haute priorité
    for (int i = 0; i < HIGH_PRIORITY_THREAD_COUNT; i++) {
        thread_create_priority(&high_priority_threads[i], task_starvation, &p_high, 0);
        if (i % 10 == 0) {
            printf("[Progress] Created %d high priority threads...\n", i);
        }
    }

    // Créer les threads de faible priorité
    for (int i = 0; i < LOW_PRIORITY_THREAD_COUNT; i++) {
        thread_create_priority(&low_priority_threads[i], task_starvation, &p_low, 4);
        printf("[Progress] Created %d low priority threads...\n", i);
    }

    // Attendre la fin des threads de haute priorité
    for (int i = 0; i < HIGH_PRIORITY_THREAD_COUNT; i++) {
        thread_join_priority(high_priority_threads[i], NULL);
        if (i % 10 == 0) {
            printf("[Progress] Joined %d high priority threads...\n", i);
        }
    }

    // Attendre la fin des threads de faible priorité
    for (int i = 0; i < LOW_PRIORITY_THREAD_COUNT; i++) {
        thread_join_priority(low_priority_threads[i], NULL);
        printf("[Progress] Joined %d low priority threads...\n", i);
    }

    printf("\n=== Stress Starvation Test Results ===\n");
    printf("High priority threads finished: %d / %d\n", high_priority_done, HIGH_PRIORITY_THREAD_COUNT);
    printf("Low priority threads finished:  %d / %d\n", low_priority_done, LOW_PRIORITY_THREAD_COUNT);
    printf("=== End of Stress Starvation Test ===\n\n");

    return 0;
}
