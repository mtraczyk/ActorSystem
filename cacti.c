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
  check_alloc_validity(begin_cyclic_messages_index = malloc(sizeof(int)));
  check_alloc_validity(end_cyclic_messages_index = malloc(sizeof(int)));

  check_alloc_validity(queue_of_actors = malloc(POOL_SIZE * sizeof(actor_id_t *)));
  check_alloc_validity(queue_of_actors_length = malloc(POOL_SIZE * sizeof(int)));
  check_alloc_validity(begin_cyclic_actors_index = malloc(POOL_SIZE * sizeof(int)));
  check_alloc_validity(end_cyclic_actors_index = malloc(POOL_SIZE * sizeof(int)));

  for (int i = 0; i < POOL_SIZE; i++)
    check_alloc_validity(queue_of_actors[i] = malloc(sizeof(actor_id_t)));

  begin_cyclic_messages_index = end_cyclic_messages_index = 0;
  for (int i = 0; i < POOL_SIZE; i++) {
    queue_of_actors_length[i] = 1;
    begin_cyclic_actors_index[i] = 0;
    end_cyclic_actors_index[i] = 0;
  }
}

static void adjust_dynamic_arrays() {

}

static void adjust_queue_of_actors() {

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