

#include "plugin/components/SoundManager.hpp"

#include "Json/JsonParser.hpp"
#include "Sound.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/Registry.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "ecs/zipper/ZipperIndex.hpp"
#include "plugin/events/DeathEvent.hpp"
#include "plugin/events/SoundEvents.hpp"

void Sound::init_sound_manager(Ecs::Entity& e, const JsonObject& obj)
{
  std::unordered_map<std::string, SoundEffect> sound_effects;

  JsonArray sound_effects_array =
      get_value<SoundManager, JsonArray>(
          this->_registry.get(), obj, e, "sound_effects")
          .value_or(JsonArray());

  try {
    for (const auto& sound_effect_value : sound_effects_array) {
      JsonObject sound_effect_obj =
          std::get<JsonObject>(sound_effect_value.value);
      if (!sound_effect_obj.contains("name")) {
        std::cerr << "Error loading SoundEffect component: missing name in "
                     "JsonObject\n";
        return;
      }
      std::string name = get_value<SoundEffect, std::string>(
                             this->_registry.get(), sound_effect_obj, e, "name")
                             .value_or("");
      if (!sound_effect_obj.contains("filepath")) {
        std::cerr << "Error loading SoundEffect component: missing filepath in "
                     "JsonObject\n";
        return;
      }
      std::string filepath =
          get_value<SoundEffect, std::string>(
              this->_registry.get(), sound_effect_obj, e, "filepath")
              .value_or("");

      if (!sound_effect_obj.contains("volume")) {
        std::cerr << "Error loading SoundEffect component: missing volume in "
                     "JsonObject\n";
        return;
      }
      double volume = get_value<SoundEffect, double>(
                          this->_registry.get(), sound_effect_obj, e, "volume")
                          .value_or(100.0);

      double pitch = 1.0;
      if (sound_effect_obj.contains("pitch")) {
        pitch = get_value<SoundEffect, double>(
                    this->_registry.get(), sound_effect_obj, e, "pitch")
                    .value_or(1.0);
      }

      bool loop = false;
      if (sound_effect_obj.contains("loop")) {
        loop = get_value<SoundEffect, bool>(
                   this->_registry.get(), sound_effect_obj, e, "loop")
                   .value_or(false);
      }
      sound_effects.insert_or_assign(name,
                                     SoundEffect(filepath,
                                                 volume,
                                                 pitch,
                                                 loop,
                                                 /*play=*/false,
                                                 /*stop=*/true,
                                                 /*playing=*/false));
    }
  } catch (std::bad_variant_access const&) {
    std::cerr << "Error parsing SoundManager component: sound_effects array "
                 "contains invalid value"
              << '\n';
    return;
  }
  init_component<SoundManager>(this->_registry.get(),
                               this->_event_manager.get(),
                               e,
                               SoundManager(std::move(sound_effects)));
}

bool Sound::on_play_sound(Registry& r, const PlaySoundEvent& event)
{
  for (const auto&& [e, sound] : ZipperIndex<SoundManager>(r)) {
    if (e == event.entity) {
      if (!sound._sound_effects.contains(event.name)) {
        return false;
      }
      SoundEffect& sound_effect = sound._sound_effects.at(event.name);

      if (sound_effect.playing) {
        return false;
      }

      // std::cout << event.volume << std::endl;;
      sound_effect.volume = event.volume;
      sound_effect.pitch = event.pitch;
      sound_effect.loop = event.loop;
      sound_effect.play = true;
      sound_effect.stop = false;
      return false;
    }
  }
  return false;
}

void Sound::sound_system(Registry& r)
{
  for (auto&& [sound] : Zipper<SoundManager>(r)) {
    for (auto& [name, sound_effect] : sound._sound_effects) {
      if (sound_effect.stop) {
        sound_effect.playing = false;
        sound_effect.stop = false;
      }
    }
  }
}

void Sound::on_death(Registry& r, EventManager& em, const DeathEvent& event)
{
  if (r.is_entity_dying(event.entity)) {
    return;
  }
  if (!r.has_component<SoundManager>(event.entity)) {
    return;
  }

  auto& sounds = r.get_components<SoundManager>();

  if (sounds[event.entity].value()._sound_effects.contains("death")
      && !sounds[event.entity].value()._sound_effects.at("death").playing)
  {
    SoundEffect& sound_effect =
        sounds[event.entity].value()._sound_effects.at("death");
    em.emit<PlaySoundEvent>(event.entity,
                            "death",
                            sound_effect.volume,
                            sound_effect.pitch,
                            sound_effect.loop);
  }
}
