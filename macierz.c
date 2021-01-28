#include "cacti.h"
#include "global.h"

void f(void **stateptr, size_t nbytes, void *data) {
  printf("HELLO WORLD\n");
}

typedef void (*act)(void **stateptr, size_t nbytes, void *data);

int main() {
  actor_id_t first;
  act prompts[1];
  prompts[0] = &f;
  role_t a = {1, prompts};
  a.nprompts = 0;
  actor_system_create(&first, &a);
  actor_system_join(first);

  return 0;
}
