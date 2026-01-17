#pragma once

#include "Json/JsonParser.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/events/MusicEvents.hpp"
#include "plugin/events/SoundEvents.hpp"

class Sound : public APlugin
{
public:
  Sound(Registry& r, EventManager& em, EntityLoader& l);

  void init_sound_manager(Ecs::Entity& e, const JsonObject& obj);
  void init_music_manager(Ecs::Entity& e, const JsonObject& obj);
  void init_master_volume(Ecs::Entity& e, const JsonObject& obj);
  void init_sfx_volume(Ecs::Entity& e, const JsonObject& obj);
  void init_music_volume(Ecs::Entity& e, const JsonObject& obj);

  bool on_play_sound(Registry& r, const PlaySoundEvent& event);
  void sound_system(Registry& r);
  bool on_play_music(Registry& r, const PlayMusicEvent& event);
  void music_system(Registry& r);

};
