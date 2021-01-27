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
  actors[0].msg_q.writepos = actors[0].msg_q.readpos = actors[0].msg_q.size = 0;

  check_alloc_validity(actor_q = malloc(POOL_SIZE * sizeof(actor_buffer)));
  for (uint32_t i = 0; i < POOL_SIZE; i++) {
    check_alloc_validity(actor_q[i].actor_id = malloc(sizeof(actor_id_t)));
    actor_q[i].size = 1;
    actor_q[i].writepos = actor_q[i].readpos = 0;
    if ((err = pthread_mutex_init(&actor_q[i].lock, 0)) != 0)
      handle_error_en(err, "pthread_mutex_init");
  }
}

void *thread_task(void *data) {
  uint32_t thread_number = *(uint32_t *) (data);

  while (is_the_system_alive) {

  }

  free(data);
}

actor_id_t actor_id_self() {

}

// Creates a brand new actor system.
int actor_system_create(actor_id_t *actor, role_t *const role) {
  int err;
  initialize();

  actors[0].id = 0;
  actors[0].role = role;
  *actor = actors[0].id;

  uint32_t *thread_number;
  for (uint32_t i = 0; i < POOL_SIZE; i++) {
    check_alloc_validity(thread_number = malloc(sizeof(uint32_t)));
    *thread_number = i;
    if ((err = pthread_create(&th[i], &attr, thread_task, thread_number)))
      handle_error_en(err, "pthread_create");
  }
}

void actor_system_join(actor_id_t actor) {
  int err;
  is_the_system_alive = false;

  for (uint32_t i = 0; i < POOL_SIZE; i++)
    if ((err = pthread_join(th[i], 0)) != 0)
      handle_error_en(err, "pthread_join");
}

int send_message(actor_id_t actor, message_t message) {

}



