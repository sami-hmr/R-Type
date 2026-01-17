#pragma once

#include "Json/JsonParser.hpp"
#include "artefacts/TemporalEffects.hpp"
#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/components/Inventory.hpp"
#include "plugin/events/CollisionEvent.hpp"

class Artefacts : public APlugin
{
public:
  Artefacts(Registry& r, EventManager& em, EntityLoader& l);
  ~Artefacts() override = default;
  EntityLoader& entity_loader;

private:
  template<typename T>
  void init_temporal_artefacts(Registry::Entity const& entity,
                               JsonObject const& obj);

  template<typename T>
  Item artefact_to_item(TemporalEffect<T> artefact,
                        std::string evt_name,
                        JsonObject evt_params);

  template<typename T>
  TemporalEffect<T> item_to_temporal_artefact(Item const& item);

  template<typename T>
  void on_temporal_effect(CollisionEvent const& event,
                          const std::string& evt_name,
                          const std::string& param_field);
};
