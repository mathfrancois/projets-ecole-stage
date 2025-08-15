#include "thread_ext.h"
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

#define INTERRUPTION_TIMER 20 // 20 ms
#define AGING_THRESHOLD 5 // Nombre de ticks avant de promouvoir
#define PREEMPTION_SIGNAL SIGUSR1

#define MAX_PRIORITY_LEVELS 5

#ifdef LOGGING_ENABLED
    #define LOG(msg, ...) fprintf(stderr, msg, ##__VA_ARGS__)
#else
    #define LOG(msg, ...)  // Ne fait rien si LOGGING_ENABLED n'est pas défini
#endif

#define STACK_SIZE (64 * 1024)

#ifndef TAILQ_FOREACH_SAFE
#define TAILQ_FOREACH_SAFE(var, head, field, tvar) \
    for ((var) = TAILQ_FIRST((head)); \
         (var) && ((tvar) = TAILQ_NEXT((var), field), 1); \
         (var) = (tvar))
#endif


enum thread_state {
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_TERMINATED
};

typedef struct {
    void *(*func)(void *);
    void *arg;
} thread_args_t;

typedef struct thread_config_t{
    int id;
    ucontext_t context;
    enum thread_state state;
    void *retval;
    thread_args_t *args;
    int priority;
    int wait_ticks; // Combien de ticks il a attendu sans exécuter 
    int valgrind_stackid;
    TAILQ_ENTRY(thread_config_t) next;
} thread_config_t;



// Initialise la liste doublement chaînée des threads prêts à l'exécution
TAILQ_HEAD(thread_list, thread_config_t);
struct thread_list terminated_queue = TAILQ_HEAD_INITIALIZER(terminated_queue); 

struct thread_list ready_queues[MAX_PRIORITY_LEVELS];




// Variable globales pour le thread courant et le compteur d'identifiant
int is_init = 0;
thread_config_t *current_thread = NULL;
int thread_id_counter = 0; // 0 = main thread
sigset_t sigset;
timer_t timerid;

void block_timer_signal_priority() {
    sigprocmask(SIG_BLOCK, &sigset, NULL);
}

void unblock_timer_signal_priority() {
    sigprocmask(SIG_UNBLOCK, &sigset, NULL);
}

void aging_update() {
    for (int p = MAX_PRIORITY_LEVELS - 1; p > 0; p--) { // Parcours les priorités basses vers hautes
        thread_config_t *t, *tmp;
        TAILQ_FOREACH_SAFE(t, &ready_queues[p], next, tmp){
            t->wait_ticks++;
            if (t->wait_ticks >= AGING_THRESHOLD) {
                TAILQ_REMOVE(&ready_queues[p], t, next);
                t->priority--;
                t->wait_ticks = 0;
                TAILQ_INSERT_TAIL(&ready_queues[t->priority], t, next);
            }
        }
    }
}

void scheduler_tick_priority() {
    block_timer_signal_priority();  
    aging_update(); // Appliquer l'aging avant de yield
    thread_yield_priority();
    unblock_timer_signal_priority();
}

void setup_preemption_priority() {
    struct sigaction sa;
    sa.sa_handler = scheduler_tick_priority;
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



void free_main_thread_at_exit_priority() {
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
        unblock_timer_signal_priority(); // Débloquer les signaux avant d'appeler la fonction
        ret = args->func(args->arg);  // appel de la fonction du thread
    } else {
        ret = NULL;
    }
    thread_exit_priority(ret); 
    
}

void init_main_thread_priority() {
    is_init = 1;

    for (int i = 0; i < MAX_PRIORITY_LEVELS; i++) {
        TAILQ_INIT(&ready_queues[i]);
    }

    current_thread = malloc(sizeof(thread_config_t));
    assert(current_thread != NULL);
    current_thread->id = thread_id_counter++;
    current_thread->state = THREAD_RUNNING;
    current_thread->retval = NULL;
    current_thread->args = NULL;
    current_thread->priority = 0; // Priorité par défaut pour le thread principal
    current_thread->wait_ticks = 0;

    // Initialiser le contexte du thread principal
    int err_get_context = getcontext(&current_thread->context);
    assert(!err_get_context);
    LOG("Main thread initialized with ID %d\n", current_thread->id);

    atexit(free_main_thread_at_exit_priority); // Enregistrement du nettoyage automatique

    setup_preemption_priority();
}


//ordonnanceur, compléxité O(5) pour l'instant, pas beaucoup de priorité donc ordonnanceur rapide
thread_config_t* pick_next_thread() {
    for (int p = 0; p < MAX_PRIORITY_LEVELS; p++) {
        if (!TAILQ_EMPTY(&ready_queues[p])) {
            return TAILQ_FIRST(&ready_queues[p]);
        }
    }
    return NULL;
}



thread_t thread_self_priority(void) {
    if (!is_init) {
        init_main_thread_priority();
    }
    return (thread_t)((intptr_t)(current_thread->id));
}

int thread_create_priority(thread_t *newthread, void *(*func)(void *), void *funcarg, int priority) {
    if (priority < 0 || priority >= MAX_PRIORITY_LEVELS) {
        fprintf(stderr, "Invalid priority level: %d\n", priority);
        return -1;
    }
    if (!is_init) {
        init_main_thread_priority();
    }

    block_timer_signal_priority();

    thread_config_t* new_thread = malloc(sizeof(thread_config_t));
    // printf("malloc for create\n");
    assert(new_thread);

    new_thread->id = thread_id_counter++;
    new_thread->retval = NULL;
    new_thread->state = THREAD_READY;
    new_thread->priority = priority;
    new_thread->wait_ticks = 0; // Initialiser le compteur d'attente à 0
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

    // Passer un seul argument de type `void *` à `thread_wrapper`
    makecontext(&new_thread->context, (void (*)(void))thread_wrapper, 1, (uintptr_t)args);

    TAILQ_INSERT_TAIL(&ready_queues[new_thread->priority], new_thread, next);
    *newthread = (void *)(intptr_t)(new_thread->id);
    LOG("Thread %d created and added to ready queue\n", new_thread->id);

    unblock_timer_signal_priority();

    return 0;
}

void print_ready_queues() {
    for (int i = 0; i < MAX_PRIORITY_LEVELS; i++) {
        thread_config_t *thread;
        printf("Priority %d: ", i);
        TAILQ_FOREACH(thread, &ready_queues[i], next) {
            printf("%d ", thread->id);
        }
        printf("\n");
    }
}

void print_terminated_queue() {
    thread_config_t *thread;
    printf("Terminated Queue: ");
    TAILQ_FOREACH(thread, &terminated_queue, next) {
        printf("%d ", thread->id);
    }
    printf("\n");
}


int thread_yield_priority(void) {
    if (!is_init) {
        init_main_thread_priority();
    }

    block_timer_signal_priority();

    aging_update();

    thread_config_t *next_thread = pick_next_thread();

    if (next_thread && next_thread != current_thread) {
        TAILQ_REMOVE(&ready_queues[next_thread->priority], next_thread, next);
        assert(next_thread->state == THREAD_READY);

        thread_config_t *old_thread = current_thread;
        old_thread->state = THREAD_READY;
        TAILQ_INSERT_TAIL(&ready_queues[old_thread->priority], old_thread, next);

        next_thread->state = THREAD_RUNNING;
        current_thread = next_thread;
        current_thread->wait_ticks = 0; // Réinitialiser le compteur d'attente

        LOG("Thread %d yielded, switching to thread %d\n", old_thread->id, current_thread->id);
        int err_swapcontext = swapcontext(&old_thread->context, &current_thread->context);
        assert(!err_swapcontext);
    }

    unblock_timer_signal_priority();
    return 0;
}



void cleanup_zombie_priority() {
    thread_config_t *temp_thread;
    thread_config_t *next_thread;
    thread_config_t *to_free = NULL;

    temp_thread = TAILQ_FIRST(&terminated_queue);
    while (temp_thread != NULL) {
        next_thread = TAILQ_NEXT(temp_thread, next);  // On garde une référence vers le prochain

        if (temp_thread->id != 0) {
            // Libérer immédiatement
            TAILQ_REMOVE(&terminated_queue, temp_thread, next);
            free(temp_thread);
        } else {
            // Thread principal — on le garde pour plus tard
            to_free = temp_thread;
        }

        temp_thread = next_thread;  // Avancer dans la liste
    }

    // On supprime le thread principal de la file mais on ne le libère pas tout de suite
    if (to_free) {
        TAILQ_REMOVE(&terminated_queue, to_free, next);
        current_thread = to_free;
    }
}

int thread_join_priority(thread_t thread, void **retval) {
    if (!is_init) {
        init_main_thread_priority();
    }

    block_timer_signal_priority();

    // Convertir thread_t en ID
    int target_id = (int)(intptr_t)thread;

    thread_config_t *target_thread = NULL;

    // Vérifier que le thread n'est pas le thread courant
    assert(current_thread->id != target_id);

    // Chercher le thread cible dans ready_queue
    // Chercher dans toutes les ready_queues
    for (int i = 0; i < MAX_PRIORITY_LEVELS; i++) {
        TAILQ_FOREACH(target_thread, &ready_queues[i], next) {
            if (target_thread->id == target_id) {
                break;
            }
        }
        if (target_thread) {
            break; // trouvé -> on arrête de chercher
        }
    }
    // Si non trouvé, chercher dans terminated_queue
    if (!target_thread) {
        TAILQ_FOREACH(target_thread, &terminated_queue, next) {
            if (target_thread->id == target_id) {
                break;
            }
        }
    }
    // Attendre que le thread termine
    while (target_thread->state != THREAD_TERMINATED) {
        unblock_timer_signal_priority();
        thread_yield_priority();
        block_timer_signal_priority();
    }

    // Récupérer la valeur de retour si demandée
    if (retval) {
        *retval = target_thread->retval;
    }
    // Si ce n’est pas le thread principal, on libère tout normalement
    if (target_thread->id != 0) {
        // Supprimer le thread de terminated_queue
        TAILQ_REMOVE(&terminated_queue, target_thread, next);
        VALGRIND_STACK_DEREGISTER(target_thread->valgrind_stackid);
        if (target_thread->args) {
            free(target_thread->args);
            target_thread->args = NULL;
        }
        free(target_thread->context.uc_stack.ss_sp);
        free(target_thread);
    }

    // On libère toujours la structure elle-même
    
    LOG("Thread %d terminated and cleaned up\n", target_id);
    unblock_timer_signal_priority();

    return 0;
}

// Modification de la fonction thread_exit
void thread_exit_priority(void *retval) {
    if (!is_init) {
        init_main_thread_priority();
    }
    
    // Blocage des signaux avant manipulation de structures partagées
    block_timer_signal_priority();

    assert(current_thread != NULL);

    // Marquer le thread comme terminé
    current_thread->state = THREAD_TERMINATED;
    current_thread->retval = retval;

    // Libérer les args
    if (current_thread->args) {
        free(current_thread->args);
        current_thread->args = NULL;
    }

    // Ajouter à la terminated_queue
    TAILQ_INSERT_TAIL(&terminated_queue, current_thread, next);
    thread_config_t *next_thread = pick_next_thread();
    if (next_thread) {
        TAILQ_REMOVE(&ready_queues[next_thread->priority], next_thread, next);
        next_thread->state = THREAD_RUNNING;

        current_thread = next_thread;
        LOG("Thread %d exiting, switching to thread %d\n", current_thread->id, next_thread->id);

        setcontext(&next_thread->context);

        assert(0);
    } else {
        cleanup_zombie_priority(); // Libérer les threads terminés
        unblock_timer_signal_priority();
        exit(0);
    }
}