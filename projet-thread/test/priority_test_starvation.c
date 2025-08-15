#include "thread_ext.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *task_starvation(void *arg) {
    int priority = *(int *)arg;
    for (int i = 0; i < 5; i++) {
        printf("[Starvation] Thread (priority %d) iteration %d\n", priority, i);
        usleep(1000); // 1ms
        thread_yield_priority();
    }
    return NULL;
}

int main() {
    thread_t high_priority_threads[5];
    thread_t low_priority_thread;
    int p_high = 0;
    int p_low = 4;

    printf("\n=== Starvation Test ===\n");

    for (int i = 0; i < 5; i++) {
        thread_create_priority(&high_priority_threads[i], task_starvation, &p_high, 0);
    }
    thread_create_priority(&low_priority_thread, task_starvation, &p_low, 4);

    for (int i = 0; i < 5; i++) {
        thread_join_priority(high_priority_threads[i], NULL);
    }
    thread_join_priority(low_priority_thread, NULL);

    printf("=== End of Starvation Test ===\n\n");
    return 0;
}
