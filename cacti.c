#include "global.h"

// Initializes global memory of the current actor system.
void initialize() {
  int err;
  number_of_actors = 1;
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

void actor_receive_message(uint32_t actor_with_message) {
  int err;
  // Here I still have the access to the actor`s info.

  if ((err = pthread_mutex_unlock(&actors[actor_with_message].lock)) != 0)
    handle_error_en(err, "pthread_mutex_unlock");
}

// Gets id of an actor with messages, updates the queue accordingly to the situation.
void get_actor_to_receive_message(actor_id_t *actor_with_message, uint32_t thread_number) {
  int err;

  *actor_with_message = actor_q[thread_number].actor_id[actor_q[thread_number].readpos];
  actor_q[thread_number].readpos =
    (actor_q[thread_number].readpos + 1) % actor_q[thread_number].size;

  // I need to obtain exclusive access to the *actor_with_message data.
  if ((err = pthread_mutex_lock(&actors[*actor_with_message].lock)) != 0)
    handle_error_en(err, "pthread_mutex_lock");

  if (actors[*actor_with_message].msg_q.number_of_messages > 1) {
    actor_q[thread_number].actor_id[actor_q[thread_number].writepos] = *actor_with_message;
    actor_q[thread_number].writepos =
      (actor_q[thread_number].writepos + 1) % actor_q[thread_number].size;
  } else {
    actor_q[thread_number].number_of_actors--;
  }
}

/* Code that POOL_SIZE threads have to execute.
*/
void *thread_task(void *data) {
  uint32_t thread_number = *(uint32_t *) (data);
  int err;
  actor_id_t actor_with_message;

  while (is_the_system_alive) {
    // Acquiring access to thread`s queue.
    if ((err = pthread_mutex_lock(&actor_q[thread_number].lock)) != 0)
      handle_error_en(err, "pthread_mutex_lock");

    while (!actor_q[thread_number].number_of_actors) {
      // Thread has nothing to do. Better for it to go to sleep.
      if ((err = pthread_cond_wait(&cond[thread_number], &actor_q[thread_number].lock)) != 0)
        handle_error_en(err, "pthread_cond_wait");
    }

    // Here, the thread does have the mutex and there is at least one actor with some messages.

    get_actor_to_receive_message(&actor_with_message, thread_number);

    // Returning access to the queue.
    if ((err = pthread_mutex_unlock(&actor_q[thread_number].lock)) != 0)
      handle_error_en(err, "pthread_mutex_unlock");

    actor_receive_message(actor_with_message);
  }

  free(data);
}

actor_id_t actor_id_self() {

}

// Creates a brand new actor system.
int actor_system_create(actor_id_t *actor, role_t *const role) {
  if (actor == NULL || role == NULL)
    return ERROR;

  int err;
  initialize();

  actors[0].id = 0;
  actors[0].role = role;
  *actor = actors[0].id;

  uint32_t *thread_number;
  for (uint32_t i = 0; i < POOL_SIZE; i++) {
    check_alloc_validity(thread_number = malloc(sizeof(uint32_t)));
    *thread_number = i;
    if ((err = pthread_create(&th[i], &attr, thread_task, thread_number)) != 0)
      handle_error_en(err, "pthread_create");
  }

  return SUCCESS;
}

void actor_system_join(actor_id_t actor) {
  int err;
  is_the_system_alive = false;

  for (uint32_t i = 0; i < POOL_SIZE; i++)
    if ((err = pthread_join(th[i], 0)) != 0)
      handle_error_en(err, "pthread_join");

#warning !CLEAN MEMORY!
}

int send_message(actor_id_t actor, message_t message) {

}



