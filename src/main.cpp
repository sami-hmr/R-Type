#include <chrono>
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include "Json/JsonParser.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/events/ActionEvents.hpp"
#include "plugin/events/SceneChangeEvent.hpp"
#include "plugin/events/ShutdownEvent.hpp"
#include "plugin/libLoaders/ILibLoader.hpp"

// exists to take controle over destroying order of Registry and EntityLoader
static int true_main(Registry& r,
                     EventManager& em,
                     EntityLoader& e,
                     const std::vector<std::string>& argv)
{
  bool should_exit = false;
  int exit_code = 0;

  em.on<ShutdownEvent>(
      "ShutdownEvent",
      [&should_exit, &exit_code](const ShutdownEvent& event) -> bool
      {
        should_exit = true;
        exit_code = event.exit_code;
        std::cout << "Shutdown requested: " << event.reason << "\n";
        return false;
      });

  em.on<SceneChangeEvent>("SceneChangeEvent",
                          [&r](const SceneChangeEvent& event) -> bool
                          {
                            std::cout << event.target_scene << std::endl;
                            if (event.force) {
                              r.deactivate_all_scenes();
                            }
                            r.activate_scene(event.target_scene);
                            return false;
                          });

      em.on<DisableSceneEvent>("DisableSceneEvent",
                          [&r](const DisableSceneEvent& event) -> bool
                          {
                            r.deactivate_scene(event.target_scene);
                            return false;
                          });


  em.on<SpawnEntityRequestEvent>(
      "SpawnEntity",
      [&r, &e](const SpawnEntityRequestEvent& event) -> bool
      {
        Registry::Entity entity = r.spawn_entity();
        JsonObject base = r.get_template(event.entity_template);
        e.load_components(entity, base);
        e.load_components(entity, event.params);
        return false;
      });

  r.init_scene_management();

  for (auto const& i : argv) {
    e.load(i);
  }

  r.setup_scene_systems();

  auto frame_duration =
      std::chrono::microseconds(1000000 / 60);  // ~33333 microseconds
  if (argv[0].contains("server")) {
    frame_duration =
        std::chrono::microseconds(1000000 / 40);  // ~33333 microseconds
  }
  auto next_frame_time = std::chrono::duration_cast<std::chrono::microseconds>(
      r.clock().now().time_since_epoch());

  while (!should_exit) {
    r.run_systems(em);

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
  std::optional<EventManager> em;
  std::optional<EntityLoader> e;

  r.emplace();
  em.emplace();
  e.emplace(*r, *em);

#ifdef RTYPE_EPITECH_CLIENT
  int result = true_main(*r, *em, *e, {"client_config"});
#elif RTYPE_EPITECH_SERVER
  int result = true_main(*r, *em, *e, {"server_config"});
#else
  int result =
      true_main(*r, *em, *e, std::vector<std::string>(argv + 1, argv + argc));
#endif

  r.reset();  // Registry first (destroys systems while plugins still loaded)
  e.reset();  // EntityLoader second (unloads plugins after systems destroyed)
  em.reset();  // EventManager last (no plugin dependencies)

  return result;
}
