#include <optional>

#include "ecs/Registery.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/libLoaders/ILibLoader.hpp"

static int true_main(Registery& r, EntityLoader& e)
{
  e.load("game_config");

  while (true) {
    r.run_systems();
  }
  return 0;
}

int main()
{
  std::optional<Registery> r;
  r.emplace();
  EntityLoader e(*r);
  true_main(*r, e);
  r.reset();
  return 0;
}
