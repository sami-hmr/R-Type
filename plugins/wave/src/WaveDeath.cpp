#include <iostream>

#include "plugin/components/Wave.hpp"

#include "WaveManager.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Scenes.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
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
      if (tag.wave_id == wave.id && !this->_registry.get().is_entity_dying(entity))
      {
        remaining++;
      }
    }
    if (remaining == 0) {
      for (auto const &event: wave.on_end) {
        if (event.event_name == "SceneChangeEvent") {
          this->_event_manager.get().emit<SceneChangeEvent>("game", "", true);
          this->_event_manager.get().emit<EventBuilderId>(
              std::nullopt,
              "SceneChangeEvent",
              SceneChangeEvent("game", "", /*f=*/true).to_bytes());

          this->_event_manager.get().emit<SceneChangeEvent>(event.event_name, "", false);
          this->_event_manager.get().emit<EventBuilderId>(
              std::nullopt,
              "SceneChangeEvent",
              SceneChangeEvent(event.event_name, "", /*f=*/false).to_bytes());
        } else {
          this->_event_manager.get().emit(
            this->_registry, event.event_name, event.params);
        }
      }
      this->_event_manager.get().emit<DeleteEntity>(wave_entity);
    }
  }
}
