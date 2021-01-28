#include "global.h"

// Initializes global memory of the current actor system.
void initialize() {
  int err;
  number_of_actors = 1;
  number_of_dead_actors = 0;
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

void obtain_message(actor_id_t actor, message_t *message) {
  // Here I have exclusive access to the actor`s info.
  *message = actors[actor].msg_q.messages[actors[actor].msg_q.readpos];
  actors[actor].msg_q.readpos = (actors[actor].msg_q.readpos + 1) % ACTOR_QUEUE_LIMIT;
  actors[actor].msg_q.number_of_messages--;
}

void receive_hello(uint32_t actor) {
  // Here I still have exclusive access to the actor`s info.

}

void receive_spawn(uint32_t actor) {
  // Here I still have exclusive access to the actor`s info.

}

void receive_godie(uint32_t actor) {
  // Here I still have exclusive access to the actor`s info.

}

void receive_standard_message(uint32_t actor) {
  // Here I still have exclusive access to the actor`s info.

}

void actor_receive_message(uint32_t actor_with_message) {
  // Here I still have exclusive access to the actor`s info.
  int err;
  message_t message;

  if (!actors[actor_with_message].is_actor_dead) {
    obtain_message(actor_with_message, &message);


    switch (message.message_type) {
      case MSG_HELLO:
        receive_hello(actor_with_message);
        break;
      case MSG_SPAWN:
        receive_spawn(actor_with_message);
        break;
      case MSG_GODIE:
        receive_godie(actor_with_message);
        break;
      default:
        receive_standard_message(actor_with_message);
    }
  }

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

    while (actor_q[thread_number].number_of_actors == 0) {
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
    return SYSTEM_CREATION_ERROR;

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

  message_t aux = {0, sizeof(NULL), NULL};
  send_message(actors[0].id, aux);

  return SYSTEM_CREATION_SUCCESS;
}

void actor_system_join(actor_id_t actor) {
  int err;
  is_the_system_alive = false;

  for (uint32_t i = 0; i < POOL_SIZE; i++)
    if ((err = pthread_join(th[i], 0)) != 0)
      handle_error_en(err, "pthread_join");

#warning !CLEAN MEMORY!
}

// If needed adjusts thread`s queue size.
void adjust_size_of_queue(uint32_t thread_number) {
  // I have exclusive access to the thread`s queue.
  if (actor_q[thread_number].number_of_actors == actor_q[thread_number].size) {
    // The queue has to be resized.
    uint32_t new_size = (actor_q[thread_number].size + 1) * MULTIPLIER / DIVIDER;
    check_alloc_validity(actor_q[thread_number].actor_id =
                           realloc(actor_q[thread_number].actor_id, new_size * sizeof(actor_id_t)));
  }
}

void add_actor_to_thread_queue(actor_id_t actor) {
  int err;
  uint32_t thread_number = actor % POOL_SIZE;

  // Acquiring access to thread`s queue.
  if ((err = pthread_mutex_lock(&actor_q[thread_number].lock)) != 0)
    handle_error_en(err, "pthread_mutex_lock");

  adjust_size_of_queue(thread_number);

  if (actor_q[thread_number].number_of_actors == 0) {
    // The thread is asleep, we have to wake it up.
    if ((err = pthread_cond_signal(&cond[thread_number])) != 0)
      handle_error_en(err, "pthread_cond_signal");
  }

  // Now there must be enough place for another actor_id_t.
  actor_q[thread_number].actor_id[actor_q[thread_number].writepos] = actor;
  actor_q[thread_number].number_of_actors++;
  actor_q[thread_number].writepos =
    (actor_q[thread_number].writepos + 1) % actor_q[thread_number].size;

  // Returning access to the queue.
  if ((err = pthread_mutex_unlock(&actor_q[thread_number].lock)) != 0)
    handle_error_en(err, "pthread_mutex_unlock");
}

int send_message(actor_id_t actor, message_t message) {
  int err;
  bool is_actor_dead, is_queue_full;

  if (actor >= number_of_actors)
    return ACTOR_ID_INCORRECT;

  // Obtaining exclusive access to the actor info.
  if ((err = pthread_mutex_lock(&actors[actor].lock)) != 0)
    handle_error_en(err, "pthread_mutex_lock");

  is_actor_dead = actors[actor].is_actor_dead;
  is_queue_full = actors[actor].msg_q.number_of_messages == ACTOR_QUEUE_LIMIT;

  if (!is_actor_dead) {
    if (actors[actor].msg_q.number_of_messages < ACTOR_QUEUE_LIMIT) {
      actors[actor].msg_q.number_of_messages++;
      actors[actor].msg_q.messages[actors[actor].msg_q.writepos] = message;
      actors[actor].msg_q.writepos = (actors[actor].msg_q.writepos + 1) % ACTOR_QUEUE_LIMIT;

      if (actors[actor].msg_q.number_of_messages == 1) {
        add_actor_to_thread_queue(actor);
      }
    }
  }

  if ((err = pthread_mutex_unlock(&actors[actor].lock)) != 0)
    handle_error_en(err, "pthread_mutex_unlock");

  if (is_actor_dead)
    return ACTOR_IS_DEAD;

  if (is_queue_full)
    return ACTOR_QUEUE_IS_FULL;

  return SEND_MESSAGE_SUCCESS;
}




