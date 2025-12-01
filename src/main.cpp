#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "Events.hpp"
#include "ecs/Registery.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/libLoaders/ILibLoader.hpp"

static constexpr int target_fps = 60;
static constexpr int frame_time_ms = 1000 / target_fps;

static int true_main(Registery& r,
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

  for (auto const& i : argv) {
    e.load(i);
  }

  while (!should_exit) {
    r.run_systems();
  }

  return exit_code;
}

int main(int argc, char* argv[])
{
  std::optional<Registery> r;
  r.emplace();
  EntityLoader e(*r);
  int result =
      true_main(*r, e, std::vector<std::string>(argv + 1, argv + argc));
  r.reset();
  return result;
}
