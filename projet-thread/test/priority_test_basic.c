#include "thread_ext.h"
#include <stdio.h>
#include <stdlib.h>

void *task_basic(void *arg) {
    int num = *(int *)arg;
    printf("[Basic] Thread %d is running\n", num);
    return NULL;
}

int main() {
    thread_t t1, t2, t3;
    int a = 1, b = 2, c = 3;

    printf("\n=== Basic Functionality Test ===\n");

    thread_create_priority(&t1, task_basic, &a, 2);
    thread_create_priority(&t2, task_basic, &b, 1);
    thread_create_priority(&t3, task_basic, &c, 0);

    thread_join_priority(t1, NULL);
    thread_join_priority(t2, NULL);
    thread_join_priority(t3, NULL);

    printf("=== End of Basic Functionality Test ===\n\n");
    return 0;
}
