#include "Moving.hpp"
#include "ecs/Registery.hpp"
#include "plugin/EntityLoader.hpp"

extern "C" {
void *entry_point(Registery &r ,EntityLoader &e) {
  return new Moving(r, e);
}
}
