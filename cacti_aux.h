#ifndef CACTI_AUX_H
#define CACTI_AUX_H

#include "cacti.h"
#include <stdlib.h>
#include <stdbool.h>

extern void initialize();

extern void clean_system_memory();

extern void update_state_of_the_system(actor_id_t actor);

extern void adjust_size_of_actors_data();

extern void adjust_size_of_queue(uint32_t thread_number);

extern void obtain_message(actor_id_t actor, message_t *message);

extern void add_actor_to_thread_queue(actor_id_t actor);

extern void *thread_task(void *data);

extern bool is_system_dead();

extern void create_new_actor(actor_id_t *new_actor, message_t message);

extern void receive_hello(actor_id_t actor, message_t message);

extern void receive_spawn(actor_id_t actor, message_t message);

extern void receive_godie(actor_id_t actor);

extern void receive_standard_message(actor_id_t actor, message_t message);

extern void actor_receive_message(actor_id_t actor_with_message);

extern void get_actor_to_receive_message(actor_id_t *actor_with_message, uint32_t thread_number);

#endif // CACTI_AUX_H
