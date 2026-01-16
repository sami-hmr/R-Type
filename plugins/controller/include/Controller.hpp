#pragma once

#include <format>
#include <map>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "Json/JsonParser.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "ecs/SparseArray.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Controllable.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/events/IoEvents.hpp"
#include "plugin/events/RebindingEvent.hpp"

class Controller : public APlugin
{
public:
  Controller(Registry& r, EventManager& em, EntityLoader& l);

private:
  void init_controller(Registry::Entity const& entity, JsonObject const& obj);

  void init_event_map(Registry::Entity const& entity,
                      JsonArray& events,
                      Controllable& result);
  void handle_key_change(Key key, bool is_pressed);
  double compute_axis(Key negative, Key positive) const;
  bool is_key_active(Key key) const;
  bool handling_press_release_binding(Registry::Entity const& entity,
                                      Controllable& result,
                                      JsonObject& event,
                                      const std::string& key_string,
                                      const std::string& description,
                                      KeyEventType event_type);
  static void rebinding(Controllable& c, Rebind event, KeyEventType event_type);

  std::map<Key, bool> _key_states;
};
