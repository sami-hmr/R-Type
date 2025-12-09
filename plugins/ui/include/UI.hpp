#pragma once

#include <optional>

#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/events/Events.hpp"
#include "plugin/events/IoEvents.hpp"

class UI : public APlugin
{
public:
  UI(Registry& r, EntityLoader& l, std::optional<JsonObject> const& config);
  ~UI() override = default;

private:
  void init_input(Registry::Entity entity, const JsonVariant& config);
  void handle_key_pressed(const KeyPressedEvent& event);
};
