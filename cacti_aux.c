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

void clean_system_memory() {
  int err;

  for (uint64_t i = 0; i < number_of_actors; i++) {
    if ((err = pthread_mutex_destroy(&actors[i].lock)) != 0)
      handle_error_en(err, "pthread_mutex_destroy");

    free(actors[i].msg_q.messages);
  }
  free(actors);

  for (uint32_t i = 0; i < POOL_SIZE; i++) {
    if ((err = pthread_mutex_destroy(&actor_q[i].lock)) != 0)
      handle_error_en(err, "pthread_mutex_destroy");

    free(actor_q[i].actor_id);
  }
  free(actor_q);
}

void update_state_of_the_system(actor_id_t actor) {
  int err;

  // I need to obtain exclusive access to the *actor_with_message data.
  if ((err = pthread_mutex_lock(&actors[actor].lock)) != 0)
    handle_error_en(err, "pthread_mutex_lock");

  // Acquiring access to global data.
  if ((err = pthread_mutex_lock(&mutex)) != 0)
    handle_error_en(err, "pthread_mutex_lock");

  if (actors[actor].is_actor_dead && actors[actor].msg_q.number_of_messages == 0) {
    // Actor has already received MSG_GODIE and has no more messages on his queue.
    number_of_dead_and_finished_actors++;
  }

  if (number_of_dead_and_finished_actors == number_of_actors) {
    // System can shut down, all actors are dead.

    is_the_system_alive = false;
    for (int i = 0; i < POOL_SIZE; i++) {
      if (i != (actor % POOL_SIZE)) {
        if ((err = pthread_cond_signal(&cond[i])) != 0)
          handle_error_en(err, "pthread_cond_signal");
      }
    }
  }

  if ((err = pthread_mutex_unlock(&mutex)) != 0)
    handle_error_en(err, "pthread_mutex_unlock");

  if ((err = pthread_mutex_unlock(&actors[actor].lock)) != 0)
    handle_error_en(err, "pthread_mutex_unlock");
}

void adjust_size_of_actors_data() {
  // Here I have access to the global data.
  if (number_of_actors == actor_info_length) {
    // Here I need to block the whole system. All the threads.
    int err;

    for (uint32_t i = 0; i < POOL_SIZE; i++) {
      if ((err = pthread_mutex_lock(&actor_q[i].lock)) != 0)
        handle_error_en(err, "pthread_mutex_lock");
    }

    actor_info_length = (actor_info_length + 1) * MULTIPLIER / DIVIDER;
    check_alloc_validity(actors = realloc(actors, actor_info_length * sizeof(actor_info)));

    for (uint32_t i = 0; i < POOL_SIZE; i++) {
      if ((err = pthread_mutex_unlock(&actor_q[i].lock)) != 0)
        handle_error_en(err, "pthread_mutex_unlock");
    }
  }

  actors[number_of_actors].msg_q.messages = malloc(ACTOR_QUEUE_LIMIT * sizeof(message_t));
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

bool is_system_dead() {
  int err;
  bool aux;

  // Acquiring access to global data.
  if ((err = pthread_mutex_lock(&mutex)) != 0)
    handle_error_en(err, "pthread_mutex_lock");

  aux = is_the_system_alive;

  if ((err = pthread_mutex_unlock(&mutex)) != 0)
    handle_error_en(err, "pthread_mutex_unlock");

  return !aux;
}

void receive_hello(actor_id_t actor, message_t message) {
  int err;

  act_t aux = actors[actor].role->prompts[message.message_type];
  if ((err = pthread_mutex_unlock(&actors[actor].lock)) != 0)
    handle_error_en(err, "pthread_mutex_unlock");

  /* If the receiver is not the first actor of the system, then message.data
   * is the pointer to some actor`s id.
   */
  aux(&actors[actor].state, message.nbytes, message.data);
}

void create_new_actor(actor_id_t *new_actor, message_t message) {
  // I have to get access to the global data.
  int err;

  if ((err = pthread_mutex_lock(&mutex)) != 0)
    handle_error_en(err, "pthread_mutex_lock");

  adjust_size_of_actors_data();

  actors[number_of_actors].id = number_of_actors;
  *new_actor = actors[number_of_actors].id;
  actors[number_of_actors].is_actor_dead = false;
  actors[number_of_actors].role = (role_t *) message.data;
  actors[number_of_actors].msg_q.number_of_messages =
  actors[number_of_actors].msg_q.readpos = actors[number_of_actors].msg_q.writepos = 0;
  if ((err = pthread_mutex_init(&actors[number_of_actors].lock, 0)) != 0)
    handle_error_en(err, "pthread_mutex_init");

  number_of_actors++;

  if (number_of_actors > CAST_LIMIT)
    exit(1);

  if ((err = pthread_mutex_unlock(&mutex)) != 0)
    handle_error_en(err, "pthread_mutex_unlock");
}

void receive_spawn(actor_id_t actor, message_t message) {
  int err;

  actor_id_t new_actor;
  create_new_actor(&new_actor, message);

  if ((err = pthread_mutex_unlock(&actors[actor].lock)) != 0)
    handle_error_en(err, "pthread_mutex_unlock");

  // Sending hello message to the new actor.
  message_t aux = {MSG_HELLO, sizeof(actor_id_t), &actors[actor].id};
  send_message(actors[new_actor].id, aux);
}

void receive_godie(actor_id_t actor) {
  int err;
  if ((err = pthread_mutex_unlock(&actors[actor].lock)) != 0)
    handle_error_en(err, "pthread_mutex_unlock");

  actors[actor].is_actor_dead = true;
}

void receive_standard_message(actor_id_t actor, message_t message) {
  int err;
  act_t aux = actors[actor].role->prompts[message.message_type];

  if ((err = pthread_mutex_unlock(&actors[actor].lock)) != 0)
    handle_error_en(err, "pthread_mutex_unlock");

  aux(&actors[actor].state, message.nbytes, message.data);
}

void actor_receive_message(actor_id_t actor_with_message) {
  // Here I still have exclusive access to the actor`s info.
  message_t message;

  obtain_message(actor_with_message, &message);

  switch (message.message_type) {
    case MSG_HELLO:
      receive_hello(actor_with_message, message);
      break;
    case MSG_SPAWN:
      receive_spawn(actor_with_message, message);
      break;
    case MSG_GODIE:
      receive_godie(actor_with_message);
      break;
    default:
      receive_standard_message(actor_with_message, message);
  }

  update_state_of_the_system(actor_with_message);
}

// Gets id of an actor with messages, updates the queue accordingly to the situation.
void get_actor_to_receive_message(actor_id_t *actor_with_message, uint32_t thread_number) {
  int err;

  *actor_with_message = actor_q[thread_number].actor_id[actor_q[thread_number].readpos];
  performing_actor[thread_number] = *actor_with_message;
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

void obtain_message(actor_id_t actor, message_t *message) {
  // Here I have exclusive access to the actor`s info.
  *message = actors[actor].msg_q.messages[actors[actor].msg_q.readpos];
  actors[actor].msg_q.readpos = (actors[actor].msg_q.readpos + 1) % ACTOR_QUEUE_LIMIT;
  actors[actor].msg_q.number_of_messages--;
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

// Code that POOL_SIZE threads have to execute.
void *thread_task(void *data) {
  uint32_t thread_number = *(uint32_t *) (data);
  int err;
  actor_id_t actor_with_message;

  while (true) {
    if (is_system_dead())
      break;

    // Acquiring access to thread`s queue.
    if ((err = pthread_mutex_lock(&actor_q[thread_number].lock)) != 0)
      handle_error_en(err, "pthread_mutex_lock");

    while (actor_q[thread_number].number_of_actors == 0) {
      // Thread has nothing to do. Better for it to go to sleep.
      if ((err = pthread_cond_wait(&cond[thread_number], &actor_q[thread_number].lock)) != 0)
        handle_error_en(err, "pthread_cond_wait");

      if (is_system_dead())
        break;
    }

    if (actor_q[thread_number].number_of_actors == 0) {
      if ((err = pthread_mutex_unlock(&actor_q[thread_number].lock)) != 0)
        handle_error_en(err, "pthread_mutex_unlock");

      break;
    }

    // Here, the thread does have the mutex and there is at least one actor with some messages.
    get_actor_to_receive_message(&actor_with_message, thread_number);

    // Returning access to the queue.
    if ((err = pthread_mutex_unlock(&actor_q[thread_number].lock)) != 0)
      handle_error_en(err, "pthread_mutex_unlock");

    actor_receive_message(actor_with_message);
  }

  free(data);

  return NULL;
}
