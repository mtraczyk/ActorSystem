#include "cacti.h"
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

// Global data for the current system.

// Attributes for the POOL of threads.
pthread_attr_t attr;

// Attribute for mutex.
pthread_mutexattr_t mutex_attr;

// Mutex - will protect global variables.
pthread_mutex_t mutex;

// Number of actors in the current system.
static actor_id_t number_of_actors = 0;

// Array of pointers to actors` roles.
static role_t **actor_roles;

/* Cyclic buffer of actors` messages.
 * Acts as a queue to be more cache friendly.
*/
static message_t **actor_messages;

// Current size of actor_roles and actor_messages array.
static int actor_roles_and_messages_length = 0;

/* begin_cyclic_messages_index, tells an actor which message should be received.
 *
 * end_cyclic_messages_index, tells the system where to save a new message.
*/
static int *begin_cyclic_messages_index, *end_cyclic_messages_index;

/* Cyclic buffer of actors that have a message to receive.
 * One for every thread. Processing actors in a queue will guarantee liveness
 * of the system.
*/
static actor_id_t **queue_of_actors;

// Current size of each queue_of_actors array.
static int *queue_of_actors_length;

// Current number of actors in each of the queues.
static int *number_of_actors_in_queue;

/* begin_cyclic_actors_index, tells a thread
 * which actor should receive a message.
 *
 * end_cyclic_actors_index, tells a thread where to save info
 * about an actor awaiting a message.
*/
static int *begin_cyclic_actors_index, *end_cyclic_actors_index;


// Constants

// Multiplier for reallocs in implementation of a vector.
static const int MULTIPLIER = 3;

// Divider for reallocs in implementation of a vector.
static const int DIVIDER = 2;


// Macros

// Macro for error handlers.
#define handle_error_en(en, msg) \
              do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

#define handle_error(msg) \
               do { perror(msg); exit(EXIT_FAILURE); } while (0)


// Checks whether an alloc returned NULL.
static inline void check_alloc_validity(void *const data) {
  if (data == NULL)
    handle_error("ALLOC ERROR!");
}

// Initializes global variables.
static void initialize_system_global_memory(actor_id_t *actor, role_t *const role) {
  number_of_actors = 1;
  actor_roles_and_messages_length = 1;
  // The first actor`s identifier.
  *actor = 0;

  check_alloc_validity(actor_roles = malloc(sizeof(role_t *)));
  actor_roles[0] = role;

  check_alloc_validity(actor_messages = malloc(sizeof(message_t *)));
  check_alloc_validity(actor_messages[0] = malloc(ACTOR_QUEUE_LIMIT * sizeof(message_t)));
  check_alloc_validity(begin_cyclic_messages_index = calloc(1, sizeof(int)));
  check_alloc_validity(end_cyclic_messages_index = calloc(1, sizeof(int)));

  check_alloc_validity(queue_of_actors = malloc(POOL_SIZE * sizeof(actor_id_t *)));
  check_alloc_validity(queue_of_actors_length = malloc(POOL_SIZE * sizeof(int)));
  check_alloc_validity(begin_cyclic_actors_index = calloc(POOL_SIZE, sizeof(int)));
  check_alloc_validity(end_cyclic_actors_index = calloc(POOL_SIZE, sizeof(int)));
  check_alloc_validity(queue_of_actors_length = calloc(POOL_SIZE, sizeof(int)));
  check_alloc_validity(number_of_actors_in_queue = calloc(POOL_SIZE, sizeof(int)));

  for (int i = 0; i < POOL_SIZE; i++)
    check_alloc_validity(queue_of_actors[i] = malloc(sizeof(actor_id_t)));

  for (int i = 0; i < POOL_SIZE; i++)
    queue_of_actors_length[i] = 1;
}

// Adjusting length of actor_roles and actor_messages.
static void adjust_dynamic_arrays() {
  // I`ve got to acquire the mutex.
  int s = pthread_mutex_lock(&mutex);
  if (s != 0)
    handle_error_en(s, "pthread_mutex");

  if (actor_roles_and_messages_length == number_of_actors) {
    actor_roles_and_messages_length = actor_roles_and_messages_length * MULTIPLIER / DIVIDER;
    check_alloc_validity(actor_roles =
                           realloc(actor_roles,
                                   actor_roles_and_messages_length * sizeof(role_t *)));
    check_alloc_validity(actor_messages =
                           realloc(actor_messages,
                                   actor_roles_and_messages_length * sizeof(message_t *)));
    check_alloc_validity(begin_cyclic_messages_index =
                           realloc(begin_cyclic_messages_index,
                                   actor_roles_and_messages_length * sizeof(int)));
    check_alloc_validity(end_cyclic_messages_index =
                           realloc(end_cyclic_messages_index,
                                   actor_roles_and_messages_length * sizeof(int)));

    for (int i = number_of_actors; i < actor_roles_and_messages_length; i++) {
      check_alloc_validity(actor_messages[i] = malloc(ACTOR_QUEUE_LIMIT * sizeof(message_t)));
      begin_cyclic_messages_index[i] = 0;
      end_cyclic_messages_index[i] = 0;
    }
  }

  s = pthread_mutex_unlock(&mutex);
  if (s != 0)
    handle_error_en(s, "pthread_mutex");
}

static void adjust_queue_of_actors(int queue_number) {
  // I`ve got to acquire the mutex.
  int s = pthread_mutex_lock(&mutex);
  if (s != 0)
    handle_error_en(s, "pthread_mutex");

  if (queue_of_actors_length[queue_number] == number_of_actors_in_queue[queue_number]) {
    queue_of_actors_length[queue_number] =
      queue_of_actors_length[queue_number] * MULTIPLIER / DIVIDER;
    check_alloc_validity(queue_of_actors[queue_number] =
                           realloc(queue_of_actors[queue_number],
                                   queue_of_actors_length[queue_number] * sizeof(actor_id_t)));
  }

  s = pthread_mutex_unlock(&mutex);
  if (s != 0)
    handle_error_en(s, "pthread_mutex");
}

actor_id_t actor_id_self() {

}

int actor_system_create(actor_id_t *actor, role_t *const role) {
  initialize_system_global_memory(actor, role);

  int s = pthread_mutex_init(&mutex, &mutex_attr);
  if (s != 0)
    handle_error_en(s, "pthread_mutexattr_init");

  s = pthread_attr_init(&attr);
  if (s != 0)
    handle_error_en(s, "pthread_attr_init");

  for (int i = 0; i < POOL_SIZE; i++) {

  }
}

void actor_system_join(actor_id_t actor) {

}

int send_message(actor_id_t actor, message_t message) {

}