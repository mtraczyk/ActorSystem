#ifndef GLOBAL_H
#define GLOBAL_H

#include "cacti.h"
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

static bool is_the_system_alive = true;
static uint64_t number_of_actors; // Number of actors in the system.
// Number of actors who are dead and have no more messages to receive.
static uint64_t number_of_dead_and_finished_actors;
static pthread_mutex_t mutex; // Mutex for access to make global data changes.
static pthread_attr_t attr; // pthread_attr_t for threads.
static pthread_t th[POOL_SIZE]; // Threads` ids.
static actor_id_t performing_actor[POOL_SIZE]; // Which actor is performing in a thread.
static pthread_cond_t cond[POOL_SIZE]; // Thread will go to sleep when it has nothing to do.

// Cyclic buffer of messages acting as a queue.
typedef struct message_buffer {
  message_t *messages; // The actual data.
  uint64_t readpos, writepos; // Positions for reading and writing.
  uint64_t number_of_messages; // Number of messages in the buffer.
} message_buffer;

// Basic info about an actor.
typedef struct actor_info {
  actor_id_t id; // Actor`s id.
  role_t *role; // Actor`s role.
  void *state; // Actor`s state.
  message_buffer msg_q; // Buffer of messages acting as a queue.
  pthread_mutex_t lock;  // Mutex ensuring exclusive access to buffer.
  bool is_actor_dead; // True if actor has received MSG_GODIE.
} actor_info;

static actor_info *actors; // An array of actors` info.
static uint64_t actor_info_length; // Length of an actors array.

// Buffer of actor_id_t acting as a queue.
typedef struct actor_buffer {
  actor_id_t *actor_id; // Actor`s id.
  pthread_mutex_t lock;  // Mutex ensuring exclusive access to buffer.
  actor_id_t readpos, writepos; // Positions for reading and writing.
  uint64_t size; // Size of the buffer.
  uint64_t number_of_actors; // Number of actors in the buffer.
} actor_buffer;

/* Actor_buffer, one for every thread acting as a queue.
 * If actor is in the buffer it means he has a message to receive.
 */
static actor_buffer *actor_q;


// Constants

// Multiplier for reallocs in implementation of a vector.
static const uint64_t MULTIPLIER = 3;

// Divider for reallocs in implementation of a vector.
static const uint64_t DIVIDER = 2;


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


// Enumeration types.

enum actor_system_create_return_codes {
  SYSTEM_CREATION_SUCCESS = 0,
  SYSTEM_CREATION_ERROR = -1
};

enum send_message_return_codes {
  SEND_MESSAGE_SUCCESS = 0,
  ACTOR_IS_DEAD = -1,
  ACTOR_ID_INCORRECT = -2,
  ACTOR_QUEUE_IS_FULL = -3
};


#endif // GLOBAL_H
