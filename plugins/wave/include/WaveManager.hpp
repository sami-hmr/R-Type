#pragma once

#include <functional>
#include <cmath>

#include "Json/JsonParser.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Wave.hpp"
#include "plugin/components/Formation.hpp"

class WaveManager : public APlugin
{
public:
  WaveManager(Registry& r, EventManager& em, EntityLoader& l);
  ~WaveManager() override = default;

private:
  void init_wave(Registry::Entity const& entity, JsonObject const& obj);
  void init_formation(Registry::Entity const& entity, JsonObject const& obj);

  void wave_spawn_system(Registry& r);
  void wave_formation_system(Registry& r);

};
