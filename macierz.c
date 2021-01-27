#include "cacti.h"
#include "global.h"

int main() {
  actor_id_t first;
  actor_system_create(&first, NULL);
  actor_system_join(first);

  return 0;
}
