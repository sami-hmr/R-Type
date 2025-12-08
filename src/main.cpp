#include <cstdint>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Scenes.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/Drawable.hpp"
#include "plugin/events/Events.hpp"
#include "plugin/libLoaders/ILibLoader.hpp"

static int true_main(Registry& r,
                     EntityLoader& e,
                     const std::vector<std::string>& argv)
{
  bool should_exit = false;
  int exit_code = 0;

  r.on<ShutdownEvent>(
      [&should_exit, &exit_code](const ShutdownEvent& event)
      {
        should_exit = true;
        exit_code = event.exit_code;
        std::cout << "Shutdown requested: " << event.reason << "\n";
      });

  r.on<SceneChangeEvent>([&r](const SceneChangeEvent& event)
                         { r.set_current_scene(event.target_scene); });

  r.on<KeyPressedEvent>(
      [&r](const KeyPressedEvent& event)
      {
        if (event.key_pressed.contains(Key::ENTER)
            && event.key_pressed.at(Key::ENTER))
        {
          r.emit<SceneChangeEvent>("game", "User pressed ENTER");
        }
        if (event.key_pressed.contains(Key::R)
            && event.key_pressed.at(Key::R)) {
          r.emit<SceneChangeEvent>("menu", "User pressed R");
        }
      });

  r.init_scene_management();

  for (auto const& i : argv) {
    e.load(i);
  }

  r.setup_scene_systems();

  // JsonObject test({{"prout", JsonValue("#prout:max")}});

  // std::cout << get_value<int>(r, test, "prout").value() << std::endl;

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
  int result =
      true_main(*r, e, std::vector<std::string>(argv + 1, argv + argc));
  r.reset();
  return result;
}
