#include "cacti.h"
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

// Global data for the current system.

// Number of actors in the current system.
actor_id_t number_of_actors = 0;

// Array of actors` roles.
role_t *actor_roles;

// Current size of actor_roles array.
int actor_roles_length = 0;

/* Cyclic buffer of actors` messages.
 * Acts as a queue to be more cache friendly.
*/
message_t **actor_messages;

// Current size of actor_messages array.
int actor_messages_length = 0;

/* begin_cyclic_messages_index, tells an actor which message should be received.
 *
 * end_cyclic_messages_index, tells the system where to save a new message.
*/
int *begin_cyclic_messages_index, *end_cyclic_messages_index;

/* Cyclic buffer of actors that have a message to receive.
 * One for every thread. Processing actors in a queue will guarantee liveness
 * of the system.
*/
actor_id_t **queue_of_actors;

// Current size of every queue_of_actors array.
int *queue_of_actors_length;

/* begin_cyclic_actors_index, tells a thread
 * which actor should receive a message.
 *
 * end_cyclic_actors_index, tells a thread where to save info
 * about an actor awaiting a message.
*/
int *begin_cyclic_actors_index, *end_cyclic_actors_index;


// Macro for error handlers.
#define handle_error_en(en, msg) \
               do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

#define handle_error(msg) \
               do { perror(msg); exit(EXIT_FAILURE); } while (0)


actor_id_t actor_id_self() {

}

int actor_system_create(actor_id_t *actor, role_t *const role) {
  int s;
  pthread_attr_t attr;

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