#pragma once

#include <format>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "plugin/events/Events.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"
#include "ecs/SparseArray.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Controllable.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Velocity.hpp"

class Controller : public APlugin
{
public:
  Controller(Registery& r, EntityLoader& l);

private:
  void init_controller(Registery::Entity const entity,
                       JsonVariant const& config);

  std::optional<Key> char_to_key(char c);
  void handle_key_change(Key key, bool is_pressed);
};
