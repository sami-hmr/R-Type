#include <iostream>
#include <optional>

#include "plugin/components/Wave.hpp"

#include "WaveManager.hpp"
#include "ecs/Entity.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Scenes.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/components/Direction.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/WaveTag.hpp"
#include "plugin/events/EntityManagementEvent.hpp"
#include "plugin/events/SceneChangeEvent.hpp"

void WaveManager::wave_death_system(Registry& r)
{
  for (auto&& [wave_entity, wave] : ZipperIndex<Wave>(r)) {
    if (!wave.tracked || !wave.spawned) {
      continue;
    }

    int remaining = 0;
    for (auto&& [entity, tag] : ZipperIndex<WaveTag>(this->_registry.get())) {
      if (tag.wave_id == wave.id
          && !this->_registry.get().is_entity_dying(entity))
      {
        remaining++;
      }
    }
    if (remaining == 0) {
      auto entity_opt = std::optional<Ecs::Entity>(wave_entity);
      for (auto const& event : wave.on_end) {
        this->_event_manager.get().emit(
            this->_registry, event.event_name, event.params, entity_opt);
        this->_event_manager.get().emit<EventBuilderId>(
            std::nullopt,
            event.event_name,
            this->_event_manager.get().get_event_with_id(
                this->_registry.get(), event.event_name, event.params, entity_opt));
      }
      this->_event_manager.get().emit<DeleteEntity>(wave_entity);
    }
  }
}
