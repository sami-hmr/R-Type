#include <vector>
#include <string>
#include <optional>
#include <iostream>

#include "ecs/Scenes.hpp"
#include "ecs/Registry.hpp"
#include "Json/JsonParser.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/events/ActionEvents.hpp"
#include "plugin/events/ShutdownEvent.hpp"
#include "plugin/libLoaders/ILibLoader.hpp"
#include "plugin/events/SceneChangeEvent.hpp"

static int true_main(Registry& r,
                     EntityLoader& e,
                     const std::vector<std::string>& argv)
{
  bool should_exit = false;
  int exit_code = 0;

  r.on<ShutdownEvent>("ShutdownEvent",
                      [&should_exit, &exit_code](const ShutdownEvent& event)
                      {
                        should_exit = true;
                        exit_code = event.exit_code;
                        std::cout << "Shutdown requested: " << event.reason
                                  << "\n";
                      });

  r.on<SceneChangeEvent>("SceneChangeEvent",
                         [&r](const SceneChangeEvent& event)
                         {
                           r.set_current_scene(
                               event.target_scene,
                               SCENE_STATE_STR.at_second(event.state));
                         });

  r.on<SpawnEntityRequestEvent>("SpawnEntity",
                                [&r, &e](const SpawnEntityRequestEvent& event)
                                {
                                  Registry::Entity entity = r.spawn_entity();
                                  JsonObject base =
                                      r.get_template(event.entity_template);
                                  e.load_components(entity, base);
                                  e.load_components(entity, event.params);
                                });

  r.init_scene_management();

  for (auto const& i : argv) {
    e.load(i);
  }

  r.setup_scene_systems();

  while (!should_exit) {
    r.run_systems();
  }

  return exit_code;
}

int main(int argc, char* argv[])
{
  std::optional<Registry> r;
  r.emplace();
  EntityLoader e(*r);
#ifdef RTYPE_EPITECH_CLIENT
  int result = true_main(*r, e, {"client_config"});
#elif RTYPE_EPITECH_SERVER
  int result = true_main(*r, e, {"server_config"});
#else
  int result =
      true_main(*r, e, std::vector<std::string>(argv + 1, argv + argc));
#endif
  r.reset();
  return result;
}
