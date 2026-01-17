#include "Sound.hpp"

#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/components/MusicManager.hpp"
#include "plugin/components/SoundManager.hpp"
#include "plugin/events/SoundEvents.hpp"

Sound::Sound(Registry& r, EventManager& em, EntityLoader& l)
    : APlugin("sound",
              r,
              em,
              l,
              {},
              {COMP_INIT(SoundManager, SoundManager, init_sound_manager),
               COMP_INIT(MusicManager, MusicManager, init_music_manager)})
{
  REGISTER_COMPONENT(SoundManager)
  REGISTER_COMPONENT(MusicManager)

  this->_registry.get().add_system(
      [this](Registry& r) { this->sound_system(r); }, 1000);
  this->_registry.get().add_system(
      [this](Registry& r) { this->music_system(r); }, 1000);

  SUBSCRIBE_EVENT(PlaySoundEvent,
                  { this->on_play_sound(this->_registry.get(), event); })
  SUBSCRIBE_EVENT(PlayMusicEvent,
                  { this->on_play_music(this->_registry.get(), event); })
}

extern "C"
{
PLUGIN_EXPORT void* entry_point(Registry& r, EventManager& em, EntityLoader& e)
{
  return new Sound(r, em, e);
}
}
