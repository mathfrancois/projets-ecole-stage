#include "thread_ext.h"  

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

void *simple_function(void *arg) {
    int id = *(int *)arg;
    printf("Simple thread running with id = %d\n", id);
    return (void *)(intptr_t)(id * 2);
}

void *busy_function() {
    volatile int counter = 0;
    while (counter < 10000000) {
        counter++;
    }
    printf("Busy thread finished.\n");
    return NULL;
}

void *high_priority_function() {
    printf("High priority thread running!\n");
    return NULL;
}

void test_basic_creation() {
    printf("=== Test: basic creation ===\n");

    thread_t t1;
    int arg1 = 1;
    assert(thread_create_priority(&t1, simple_function, &arg1, 2) == 0);

    void *ret;
    thread_join_priority(t1, &ret);
    assert((intptr_t)ret == 2);
}

void test_preemption() {
    printf("=== Test: preemption ===\n");

    thread_t t1, t2;
    thread_create_priority(&t1, busy_function, NULL, 3);
    thread_create_priority(&t2, busy_function, NULL, 3);

    void *ret;
    thread_join_priority(t1, &ret);
    thread_join_priority(t2, &ret);
}

void test_priority_ordering() {
    printf("=== Test: priority ordering ===\n");

    thread_t t1, t2;
    thread_create_priority(&t1, busy_function, NULL, 4); // low priority
    thread_create_priority(&t2, high_priority_function, NULL, 0); // highest priority

    void *ret;
    thread_join_priority(t1, &ret);
    thread_join_priority(t2, &ret);
}

int main() {
    test_basic_creation();
    test_preemption();
    test_priority_ordering();
    printf("All tests passed!\n");
    return 0;
}
