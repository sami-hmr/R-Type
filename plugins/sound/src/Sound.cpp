#include "Sound.hpp"

#include "ecs/Registry.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/components/Sound.hpp"
#include "plugin/events/SoundEvents.hpp"

Sound::Sound(Registry& r, EventManager& em, EntityLoader& l)
    : APlugin("sound",
              r,
              em,
              l,
              {},
              {COMP_INIT(SoundManager, SoundManager, init_sound_manager)})
{
  REGISTER_COMPONENT(SoundManager)

  this->_registry.get().add_system(
      [this](Registry& r) { this->sound_system(r); }, 1000);

  SUBSCRIBE_EVENT(PlaySoundEvent,
                  { this->on_play_sound(this->_registry.get(), event); })
}

extern "C"
{
void* entry_point(Registry& r, EventManager& em, EntityLoader& e)
{
  return new Sound(r, em, e);
}
}
