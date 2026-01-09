#include <iostream>

#include "plugin/components/Wave.hpp"

#include "WaveManager.hpp"
#include "ecs/Registry.hpp"
#include "ecs/Scenes.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "plugin/components/Direction.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/WaveTag.hpp"
#include "plugin/events/EntityManagementEvent.hpp"

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
      std::cout << "START EVENT" << std::endl;
      this->_event_manager.get().emit(
          this->_registry, wave.on_end.event_name, wave.on_end.params);
      std::cout << "DEATH EVENT" << std::endl;
      this->_event_manager.get().emit<DeleteEntity>(wave_entity);
    }
  }
}
