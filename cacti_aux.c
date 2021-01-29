#include "cacti_aux.h"
#include "global.h"

/* Definition of global variables.
 */

bool is_the_system_alive; // True when the system can shut down.
uint64_t number_of_actors; // Number of actors in the system.
uint64_t number_of_dead_and_finished_actors; // Actors that have empty queues and are dead.
pthread_mutex_t mutex; // Mutex for access to make global data changes.
pthread_attr_t attr; // pthread_attr_t for threads.
pthread_t th[POOL_SIZE]; // Threads` ids.
actor_id_t performing_actor[POOL_SIZE]; // Which actor is performing in a thread.
pthread_cond_t cond[POOL_SIZE]; // Thread will go to sleep when it has nothing to do.

actor_info *actors; // An array of actors` info.
uint64_t actor_info_length; // Length of an actors array.

/* Actor_buffer, one for every thread acting as a queue.
 * If actor is in the buffer it means he has a message to receive.
 */
actor_buffer *actor_q;

// Initializes global memory of the current actor system.
void initialize() {
  int err;
  is_the_system_alive = true;
  number_of_actors = 1;
  number_of_dead_and_finished_actors = 0;
  actor_info_length = 1;
  if ((err = pthread_mutex_init(&mutex, 0)) != 0)
    handle_error_en(err, "pthread_mutex_init");
  if ((err = pthread_attr_init(&attr)) != 0)
    handle_error_en(err, "pthread_attr_init");

  check_alloc_validity(actors = malloc(sizeof(actor_info)));
  check_alloc_validity(actors[0].msg_q.messages = malloc(ACTOR_QUEUE_LIMIT * sizeof(message_t)));
  if ((err = pthread_mutex_init(&actors[0].lock, 0)) != 0)
    handle_error_en(err, "pthread_mutex_init");
  actors[0].msg_q.writepos = actors[0].msg_q.readpos = actors[0].msg_q.number_of_messages = 0;
  actors[0].is_actor_dead = false;

  check_alloc_validity(actor_q = malloc(POOL_SIZE * sizeof(actor_buffer)));
  for (uint32_t i = 0; i < POOL_SIZE; i++) {
    check_alloc_validity(actor_q[i].actor_id = malloc(sizeof(actor_id_t)));
    actor_q[i].size = 1;
    actor_q[i].number_of_actors = 0;
    actor_q[i].writepos = actor_q[i].readpos = 0;
    if ((err = pthread_mutex_init(&actor_q[i].lock, 0)) != 0)
      handle_error_en(err, "pthread_mutex_init");

    if ((err = pthread_cond_init(&cond[i], 0)) != 0)
      handle_error_en(err, "pthread_cond_init");
  }
}