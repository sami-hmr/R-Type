#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include <optional>
#include <iostream>

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
                             std::cout << event.target_scene << std::endl;;
                             if (event.force) {
                                 r.remove_all_scenes();
                             }
                           r.set_current_scene(
                               event.target_scene);
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

  const auto frame_duration = std::chrono::microseconds(1000000 / 120); // ~33333 microseconds
  auto next_frame_time = std::chrono::duration_cast<std::chrono::microseconds>(
      r.clock().now().time_since_epoch());

  while (!should_exit) {
      r.run_systems();

      // Calculate when the next frame should start
      next_frame_time += frame_duration;

      // Get current time
      auto current_time = std::chrono::duration_cast<std::chrono::microseconds>(
          r.clock().now().time_since_epoch());

      // Sleep until next frame time
      if (next_frame_time > current_time) {
          auto sleep_duration = next_frame_time - current_time;
          std::this_thread::sleep_for(std::chrono::microseconds(sleep_duration));
      } else {
          // Frame took too long, reset timing to avoid catch-up spiral
          next_frame_time = current_time;
      }
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
