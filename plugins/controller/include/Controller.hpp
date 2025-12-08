#pragma once

#include <format>
#include <map>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "ecs/SparseArray.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Controllable.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Velocity.hpp"
#include "plugin/events/Events.hpp"

class Controller : public APlugin
{
public:
  Controller(Registry& r, EntityLoader& l);

private:
  void init_controller(Registry::Entity const entity, JsonObject const& obj);

  Key char_to_key(char c);
  void handle_key_change(Key key, bool is_pressed);
  double compute_axis(Key negative, Key positive) const;
  bool is_key_active(Key key) const;

  std::map<Key, bool> _key_states;
};
