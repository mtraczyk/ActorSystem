#include "global.h"
#include "cacti_aux.h"

actor_id_t actor_id_self() {
  int err;
  pthread_t thread_id = pthread_self();
  actor_id_t actor_id_aux;
  // Acquiring access to global data.
  if ((err = pthread_mutex_lock(&mutex)) != 0)
    handle_error_en(err, "pthread_mutex_lock");

  for (uint32_t i = 0; i < POOL_SIZE; i++) {
    if (th[i] == thread_id) {
      actor_id_aux = performing_actor[i];
      break;
    }
  }

  if ((err = pthread_mutex_unlock(&mutex)) != 0)
    handle_error_en(err, "pthread_mutex_unlock");

  return actor_id_aux;
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

  message_t aux = {MSG_HELLO, sizeof(NULL), NULL};
  send_message(actors[0].id, aux);

  return SYSTEM_CREATION_SUCCESS;
}

void actor_system_join(actor_id_t actor) {
  int err;

  if (actor >= (int64_t) number_of_actors)
    exit(1);

  for (uint32_t i = 0; i < POOL_SIZE; i++)
    if ((err = pthread_join(th[i], 0)) != 0)
      handle_error_en(err, "pthread_join");

  clean_system_memory();
}

int send_message(actor_id_t actor, message_t message) {
  int err;
  bool is_actor_dead, is_queue_full, is_id_incorrect;

  if ((err = pthread_mutex_lock(&mutex)) != 0)
    handle_error_en(err, "pthread_mutex_lock");

  is_id_incorrect = (actor >= (int64_t) number_of_actors);

  if ((err = pthread_mutex_unlock(&mutex)) != 0)
    handle_error_en(err, "pthread_mutex_unlock");

  if (is_id_incorrect)
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




