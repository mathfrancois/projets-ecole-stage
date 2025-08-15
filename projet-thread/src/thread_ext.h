#define __THREAD_EXT_H__

#ifndef USE_PTHREAD
#include <sys/queue.h>

/* identifiant de thread
 * NB: pourra être un entier au lieu d'un pointeur si ca vous arrange,
 *     mais attention aux inconvénient des tableaux de threads
 *     (consommation mémoire, cout d'allocation, ...).
 */
typedef void * thread_t;

/* recuperer l'identifiant du thread courant.
 */
extern thread_t thread_self_priority(void);

/* creer un nouveau thread qui va exécuter la fonction func avec l'argument funcarg.
 * renvoie 0 en cas de succès, -1 en cas d'erreur.
 */
extern int thread_create_priority(thread_t *newthread, void *(*func)(void *), void *funcarg, 
                         int priority);

/* passer la main à un autre thread.
 */
extern int thread_yield_priority(void);

/* attendre la fin d'exécution d'un thread.
 * la valeur renvoyée par le thread est placée dans *retval.
 * si retval est NULL, la valeur de retour est ignorée.
 */
extern int thread_join_priority(thread_t thread, void **retval);

/* terminer le thread courant en renvoyant la valeur de retour retval.
 * cette fonction ne retourne jamais.
 *
 * L'attribut noreturn aide le compilateur à optimiser le code de
 * l'application (élimination de code mort). Attention à ne pas mettre
 * cet attribut dans votre interface tant que votre thread_exit()
 * n'est pas correctement implémenté (il ne doit jamais retourner).
 */
extern void thread_exit_priority(void *retval) __attribute__ ((__noreturn__));

#endif /* __THREAD_EXT_H__ */