#pragma once

#include "Json/JsonParser.hpp"
#include "ecs/Registery.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/ActionTrigger.hpp"
#include "plugin/events/Events.hpp"

class Actions : public APlugin
{
public:
  Actions(Registery& r, EntityLoader& l);
  ~Actions() override = default;

private:
  void init_action_trigger(Registery::Entity entity, JsonObject const& obj);

  void process_triggers();
  void execute_action(Registery::Entity entity, const Action& action);

  void on_collision(const CollisionEvent& event);
  void on_key_pressed(const KeyPressedEvent& event);
};
