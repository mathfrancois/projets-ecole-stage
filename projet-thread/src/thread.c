#include "thread.h"
#include <sys/queue.h>
#include <ucontext.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <assert.h>
#include <string.h>
#include <valgrind/valgrind.h>


#include <signal.h>
#include <time.h>

#define INTERRUPTION_TIMER 20 // 20ms
#define PREEMPTION_SIGNAL SIGUSR1

#ifdef LOGGING_ENABLED
    #define LOG(msg, ...) fprintf(stderr, msg, ##__VA_ARGS__)
#else
    #define LOG(msg, ...)  // Ne fait rien si LOGGING_ENABLED n'est pas défini
#endif

#define STACK_SIZE (64 * 1024)


enum thread_state {
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_BLOCKED,
    THREAD_TERMINATED
};

typedef struct {
    void *(*func)(void *);
    void *arg;
} thread_args_t;


typedef struct thread_config_t {
    int id;
    ucontext_t context;
    enum thread_state state;
    void *retval;
    thread_args_t *args;
    int valgrind_stackid;

    // Un champ par queue
    TAILQ_ENTRY(thread_config_t) ready_next;
    TAILQ_ENTRY(thread_config_t) all_next;
    TAILQ_ENTRY(thread_config_t) terminated_next;
    TAILQ_ENTRY(thread_config_t) waiting_next;

    TAILQ_HEAD(waiting_queue_head, thread_config_t) waiting_queue;
} thread_config_t;




// Initialise la liste doublement chaînée des threads prêts à l'exécution
TAILQ_HEAD(thread_list, thread_config_t);
struct thread_list ready_queue = TAILQ_HEAD_INITIALIZER(ready_queue); 
struct thread_list terminated_queue = TAILQ_HEAD_INITIALIZER(terminated_queue);
struct thread_list all_threads = TAILQ_HEAD_INITIALIZER(all_threads); // Liste de tous les threads



// Variable globales pour le thread courant et le compteur d'identifiant
int is_init = 0;
thread_config_t *current_thread = NULL;
int thread_id_counter = 0; // 0 = main thread
sigset_t sigset;
timer_t timerid;

#ifdef PREEMPTION_ENABLED
void block_timer_signal() {
    sigprocmask(SIG_BLOCK, &sigset, NULL);
}    

void unblock_timer_signal() {
    sigprocmask(SIG_UNBLOCK, &sigset, NULL);
}
#else
void block_timer_signal() {}
void unblock_timer_signal() {}
#endif

#ifdef PREEMPTION_ENABLED

void scheduler_tick() {
    if (!TAILQ_EMPTY(&ready_queue)) {
        thread_yield();
    }     
}

void setup_preemption() {
    struct sigaction sa;
    sa.sa_handler = scheduler_tick;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, PREEMPTION_SIGNAL);
    sa.sa_flags = SA_RESTART;

    if (sigaction(PREEMPTION_SIGNAL, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    sigemptyset(&sigset);
    sigaddset(&sigset, PREEMPTION_SIGNAL);

    // Création du timer POSIX
    struct sigevent sev;
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = PREEMPTION_SIGNAL;
    sev.sigev_value.sival_ptr = &timerid;

    if (timer_create(CLOCK_REALTIME, &sev, &timerid) == -1) {
        perror("timer_create");
        exit(EXIT_FAILURE);
    }

    struct itimerspec its;
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = INTERRUPTION_TIMER * 1000000;  // Première interruption après 20ms
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = INTERRUPTION_TIMER * 1000000;  // Périodique toutes les 20ms

    if (timer_settime(timerid, 0, &its, NULL) == -1) {
        perror("timer_settime");
        exit(EXIT_FAILURE);
    }
}

#endif

void cleanup_zombie() {
    thread_config_t *temp_thread;
    thread_config_t *next_thread;
    thread_config_t *to_free = NULL;

    temp_thread = TAILQ_FIRST(&terminated_queue);
    while (temp_thread != NULL) {
        next_thread = TAILQ_NEXT(temp_thread, terminated_next);  // On garde une référence vers le prochain

        if (temp_thread->id != 0) {
            // Libérer immédiatement
            VALGRIND_STACK_DEREGISTER(temp_thread->valgrind_stackid);

            free(temp_thread->context.uc_stack.ss_sp);
            temp_thread->context.uc_stack.ss_sp = NULL;
        
            TAILQ_REMOVE(&terminated_queue, temp_thread, terminated_next);
            TAILQ_REMOVE(&all_threads, temp_thread, all_next);
            free(temp_thread);
        } else {
            // Thread principal — on le garde pour plus tard
            to_free = temp_thread;
        }

        temp_thread = next_thread;  // Avancer dans la liste
    }

    // On supprime le thread principal de la file mais on ne le libère pas tout de suite
    if (to_free) {
        TAILQ_REMOVE(&terminated_queue, to_free, terminated_next);
        TAILQ_REMOVE(&all_threads, to_free, all_next); // On retire aussi de all_threads
        current_thread = to_free;
    }
}




void free_main_thread_at_exit() {
    if (current_thread && current_thread->id == 0) {
        LOG("Freeing main thread\n");
        free(current_thread);
        current_thread = NULL;
    }
}


static void thread_wrapper(uintptr_t args_ptr) {
    thread_args_t *args = (thread_args_t *)args_ptr;
    void *ret;
    if (args && args->func) {
        LOG("Thread %d: Executing function\n", current_thread->id);
        unblock_timer_signal(); // Débloquer les signaux avant d'appeler la fonction
        ret = args->func(args->arg);  // appel de la fonction du thread
    } else {
        ret = NULL;
    }
    thread_exit(ret); 
    
}

void init_main_thread() {
    is_init = 1;

    current_thread = malloc(sizeof(thread_config_t));
    assert(current_thread != NULL);
    current_thread->id = thread_id_counter++;
    current_thread->state = THREAD_RUNNING;
    current_thread->retval = NULL;
    current_thread->args = NULL;
    TAILQ_INIT(&current_thread->waiting_queue);
    TAILQ_INSERT_TAIL(&all_threads, current_thread, all_next);

    // Initialiser le contexte du thread principal
    int err_get_context = getcontext(&current_thread->context);
    assert(!err_get_context);
    LOG("Main thread initialized with ID %d\n", current_thread->id);

    atexit(free_main_thread_at_exit); // Enregistrement du nettoyage automatique
    atexit(cleanup_zombie); // Enregistrement du nettoyage automatique

    #ifdef PREEMPTION_ENABLED
        setup_preemption();
    #endif
}



thread_t thread_self(void) {
    if (!is_init) {
        init_main_thread();
    }
    return (thread_t)((intptr_t)(current_thread->id));
}

int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg) {
    if (!is_init) {
        init_main_thread();
    }

    block_timer_signal();

    thread_config_t* new_thread = malloc(sizeof(thread_config_t));
    assert(new_thread);

    new_thread->id = thread_id_counter++;
    new_thread->retval = NULL;
    new_thread->state = THREAD_READY;
    new_thread->context.uc_stack.ss_size = STACK_SIZE;
    new_thread->context.uc_link = NULL;
    new_thread->context.uc_stack.ss_flags = 0;
    new_thread->context.uc_stack.ss_sp = malloc(STACK_SIZE);
    assert(new_thread->context.uc_stack.ss_sp != NULL);
    memset(new_thread->context.uc_stack.ss_sp, 0, STACK_SIZE); // Initialiser la pile

    int err_get_context = getcontext(&(new_thread->context));
    assert(!err_get_context);
    assert(new_thread->context.uc_stack.ss_sp != NULL);
    new_thread->valgrind_stackid = VALGRIND_STACK_REGISTER(new_thread->context.uc_stack.ss_sp,
        new_thread->context.uc_stack.ss_sp + new_thread->context.uc_stack.ss_size);

    // Stocker les arguments dans une structure
    thread_args_t *args = malloc(sizeof(thread_args_t));
    assert(args);
    
    new_thread->args = args;
    args->func = func;
    args->arg = funcarg; 


    TAILQ_INIT(&new_thread->waiting_queue);

    // Passer un seul argument de type `void *` à `thread_wrapper`
    makecontext(&new_thread->context, (void (*)(void))thread_wrapper, 1, (uintptr_t)args);


    TAILQ_INSERT_TAIL(&ready_queue, new_thread, ready_next);
    TAILQ_INSERT_TAIL(&all_threads, new_thread, all_next);
    *newthread = (void *)(intptr_t)(new_thread->id);
    LOG("Thread %d created and added to ready queue\n", new_thread->id);

    unblock_timer_signal();

    return 0;
}


int thread_yield(void) {
    if (!is_init) {
        init_main_thread();
    }

    block_timer_signal(); 

    if (TAILQ_EMPTY(&ready_queue)) {
        unblock_timer_signal(); 
        return 0;
    }

    // Sauvegarde le thread courant (marque THREAD_READY) et le réinsère dans la file des prêts
    thread_config_t *old_thread = current_thread;

    assert(old_thread->state == THREAD_RUNNING);
    old_thread->state = THREAD_READY;
    TAILQ_INSERT_TAIL(&ready_queue, old_thread, ready_next);

    current_thread = TAILQ_FIRST(&ready_queue);

    TAILQ_REMOVE(&ready_queue, current_thread, ready_next);
    assert(current_thread->state == THREAD_READY);
    current_thread->state = THREAD_RUNNING;

    LOG("Thread %d yielded, switching to thread %d\n", old_thread->id, current_thread->id);

    // Faire le swap de contexte avec le thread courant
    if (current_thread != old_thread) {
        int err_swapcontext = swapcontext(&old_thread->context, &current_thread->context);
        assert(!err_swapcontext);
    }
    unblock_timer_signal(); 


    return 0;
}

int thread_yield_blocked(void) {

    if (!is_init) {
        init_main_thread();
    }
    block_timer_signal();

    if (TAILQ_EMPTY(&ready_queue)) {
        unblock_timer_signal(); 
        return 0;
    }

    // Sauvegarde le thread courant (marque THREAD_READY) et le réinsère dans la file des prêts
    thread_config_t *old_thread = current_thread;

    current_thread = TAILQ_FIRST(&ready_queue);

    TAILQ_REMOVE(&ready_queue, current_thread, ready_next);
    assert(current_thread->state == THREAD_READY);
    current_thread->state = THREAD_RUNNING;

    LOG("Thread %d is blocked, switching to thread %d\n", old_thread->id, current_thread->id);

    // Changer de contexte sans appeler thread_yield
    int err = swapcontext(&old_thread->context, &current_thread->context);
    assert(!err);
    unblock_timer_signal();
    return 0;
}





int thread_join(thread_t thread, void **retval) {
    if (!is_init) {
        init_main_thread();
    }
    block_timer_signal();

    int target_id = (int)(intptr_t)thread;
    assert(current_thread->id != target_id); // Ne pas attendre soi-même

    thread_config_t *target_thread = NULL;

    // Rechercher le thread cible dans la liste de tous les threads
    TAILQ_FOREACH(target_thread, &all_threads, all_next) {
        if (target_thread->id == target_id) {
            break;
        }
    }


    if (!target_thread) {
        unblock_timer_signal();
        return -1; // Thread non trouvé
    }

    if (target_thread->state == THREAD_READY || target_thread->state == THREAD_BLOCKED) {
        // Le thread cible n'est pas encore terminé : on bloque le courant
        current_thread->state = THREAD_BLOCKED;
        TAILQ_INSERT_TAIL(&target_thread->waiting_queue, current_thread, waiting_next);
        LOG("Thread %d is now waiting for thread %d\n", current_thread->id, target_id);

        thread_yield_blocked(); // Passage de main (réveil lorsque target terminé)
    }

    if (target_thread->state == THREAD_TERMINATED) {
        if (retval) {
            *retval = target_thread->retval;
        }

        // Libérer les ressources si ce n'est pas le thread principal
        if (target_thread->id != 0) {
            TAILQ_REMOVE(&terminated_queue, target_thread, terminated_next);
            TAILQ_REMOVE(&all_threads, target_thread, all_next); // On retire aussi de all_threads
            VALGRIND_STACK_DEREGISTER(target_thread->valgrind_stackid);

            free(target_thread->context.uc_stack.ss_sp);
            target_thread->context.uc_stack.ss_sp = NULL;
            free(target_thread);
            target_thread = NULL;
        }

        unblock_timer_signal();
        return 0;
    }

    unblock_timer_signal();
    return 0;
}



// Modification de la fonction thread_exit
void thread_exit(void *retval) {
    if (!is_init) {
        init_main_thread();
    }

    // Blocage des signaux avant manipulation de structures partagées
    block_timer_signal();

    assert(current_thread != NULL);

    // Marquer le thread comme terminé
    current_thread->state = THREAD_TERMINATED;
    current_thread->retval = retval;

    // Libérer les args
    if (current_thread->args) {
        free(current_thread->args);
        current_thread->args = NULL;
    }

    // Réveiller les threads en attente
    thread_config_t *waiting;
    while (!TAILQ_EMPTY(&current_thread->waiting_queue)) {
        waiting = TAILQ_FIRST(&current_thread->waiting_queue);
        TAILQ_REMOVE(&current_thread->waiting_queue, waiting, waiting_next);

        waiting->state = THREAD_READY;
        waiting->retval = retval;

        TAILQ_INSERT_TAIL(&ready_queue, waiting, ready_next);
    }

    TAILQ_INSERT_TAIL(&terminated_queue, current_thread, terminated_next);

    // S'il reste des threads prêts, passer la main
    if (!TAILQ_EMPTY(&ready_queue)) {
        thread_config_t *next_thread = TAILQ_FIRST(&ready_queue);
        TAILQ_REMOVE(&ready_queue, next_thread, ready_next);
        next_thread->state = THREAD_RUNNING;

        // Sauvegarder l'ancien thread et régler le nouveau thread courant AVANT setcontext
        LOG("Thread %d exiting, switching to thread %d\n", current_thread->id, next_thread->id);
        current_thread = next_thread;
        setcontext(&next_thread->context);
        
        // Ne doit jamais être atteint
        assert(0);
    } else {
        unblock_timer_signal();
        exit(0);
    }
}

int thread_mutex_init(thread_mutex_t *mutex) {
    if (!mutex) return -1;

    block_timer_signal();

    mutex->locked = 0;
    mutex->owner = (thread_t)0;
    TAILQ_INIT(&mutex->waiting_thread);
    LOG("Mutex initialized\n");

    unblock_timer_signal();
    return 0;
}

int thread_mutex_lock(thread_mutex_t *mutex) {
    if (!is_init) init_main_thread();
    if (!mutex) return -1;

    block_timer_signal();

    // Vérifier si le thread courant est déjà le propriétaire
    if (mutex->owner == thread_self()) {
        unblock_timer_signal();
        return -1; // Deadlock
    }

    // Si le mutex est déjà verrouillé, ajouter le thread courant à la file d'attente
    while (mutex->locked) {
        current_thread->state = THREAD_BLOCKED;
        TAILQ_INSERT_TAIL(&mutex->waiting_thread, current_thread, waiting_next);
        LOG("Thread %d is waiting for the mutex\n", current_thread->id);

        thread_yield_blocked(); // Passer la main à un autre thread
    }

    // Verrouiller le mutex
    mutex->locked = 1;
    mutex->owner = thread_self();
    LOG("Thread %d locked the mutex\n", current_thread->id);

    unblock_timer_signal();
    return 0;
}

int thread_mutex_unlock(thread_mutex_t *mutex) {
    if (!is_init) init_main_thread();
    if (!mutex) return -1;

    block_timer_signal();

    // Vérifier si le thread courant est le propriétaire
    if (mutex->owner != thread_self()) {
        unblock_timer_signal();
        return -1; // Seul le propriétaire peut déverrouiller
    }

    // Déverrouiller le mutex
    mutex->locked = 0;
    mutex->owner = (thread_t)0;
    LOG("Thread %d unlocked the mutex\n", current_thread->id);

    // Réveiller le premier thread en attente, s'il y en a un
    if (!TAILQ_EMPTY(&mutex->waiting_thread)) {
        thread_config_t *waiting_thread = TAILQ_FIRST(&mutex->waiting_thread);
        TAILQ_REMOVE(&mutex->waiting_thread, waiting_thread, waiting_next);

        waiting_thread->state = THREAD_READY;
        TAILQ_INSERT_TAIL(&ready_queue, waiting_thread, ready_next);
        LOG("Thread %d is now ready after waiting for the mutex\n", waiting_thread->id);
    }

    unblock_timer_signal();
    return 0;
}

int thread_mutex_destroy(thread_mutex_t *mutex) {
    if (!mutex) return -1;
    
    // Ne pas détruire si des threads attendent
    if (!TAILQ_EMPTY(&mutex->waiting_thread)) {
        return -1;
    }

    LOG("Mutex destroyed\n");
    return 0;
}