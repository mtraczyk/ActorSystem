#include "cacti.h"
#include "global.h"

int main() {
  actor_id_t first;
  role_t a;
  a.nprompts = 0;
  actor_system_create(&first, &a);
  actor_system_join(first);

  return 0;
}
