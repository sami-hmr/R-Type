#pragma once

#include "Json/JsonParser.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/events/InteractionZoneEvent.hpp"

class Mob : public APlugin
{
public:
  Mob(Registry& r, EventManager& em, EntityLoader& l);
  ~Mob() override = default;
  EntityLoader& entity_loader;

private:
  void init_enemy(Registry::Entity const& entity, JsonObject const& obj);
  void init_spawner(Registry::Entity const& entity, JsonObject const& obj);
  void init_parasite(Registry::Entity const& entity, JsonObject const& obj);

  void on_interaction_zone(const InteractionZoneEvent& event);
  void spawner_system(Registry& r);
  void parasite_system(Registry& r);
};
