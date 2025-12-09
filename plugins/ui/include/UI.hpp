#pragma once

#include <optional>

#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/events/Events.hpp"

class UI : public APlugin
{
public:
  UI(Registery& r, EntityLoader& l, std::optional<JsonObject> const& config);
  ~UI() override = default;

private:
  void init_input(Registery::Entity entity, const JsonVariant& config);
  void handle_key_pressed(const KeyPressedEvent& event);
};
