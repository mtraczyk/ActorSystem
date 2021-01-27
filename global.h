#ifndef GLOBAL_H
#define GLOBAL_H

#include "cacti.h"
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

extern uint32_t number_of_actors; // Number of actors in the system.

// Cyclic buffer of messages acting as a queue.
typedef struct message_buffer {
  message_t *messages; // The actual data.
  int readpos, writepos; // Positions for reading and writing.
  int size; // Number of messages in the messages array.
} message_buffer;

// Basic info about an actor.
typedef struct actor_info {
  actor_id_t id; // Actor`s id.
  role_t *role; // Actor`s role.
  void *state; // Actor`s state.
  message_buffer msg_q; // Buffer of messages acting as a queue.
  pthread_mutex_t lock;  // Mutex ensuring exclusive access to buffer.
} actor_info;

extern actor_info *actors; // An array of actors` info.
extern uint32_t actor_info_length; // Length of an actors array.

// Buffer of actor_id_t acting as a queue.
typedef struct actor_buffer {
  actor_id_t *actor_id; // Actor`s id.
  pthread_mutex_t lock;  // Mutex ensuring exclusive access to buffer.
  int readpos, writepos; // Positions for reading and writing.
  int size; // Size of the buffer.
} actor_buffer;

extern actor_buffer *actor_q[POOL_SIZE]; // Actor_buffer, one for every thread acting as a queue.


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


#endif // GLOBAL_H
