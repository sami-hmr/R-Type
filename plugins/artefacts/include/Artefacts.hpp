#pragma once

#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/components/Inventory.hpp"
#include "plugin/components/PickableTools.hpp"
#include "plugin/events/CollisionEvent.hpp"

class Artefacts : public APlugin
{
public:
  Artefacts(Registry& r, EventManager& em, EntityLoader& l);
  ~Artefacts() override = default;
  EntityLoader& entity_loader;

private:
  void init_pickable_artefacts(Registry::Entity const& entity,
                               JsonObject const& obj);

  static Item artefact_to_item(PickableTool artefact);

  void on_pickable_interaction(CollisionEvent const& event);
};
