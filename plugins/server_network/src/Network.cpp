#include "Network.hpp"

#include "ecs/Registery.hpp"
#include "plugin/EntityLoader.hpp"

extern "C"
{
void* entry_point(Registery& r, EntityLoader& e)
{
  return new NetworkServer(r, e);
}
}
