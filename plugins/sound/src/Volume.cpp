#include "plugin/components/Volume.hpp"

#include "Sound.hpp"
#include "ecs/InitComponent.hpp"

void Sound::init_master_volume(Registry::Entity& e, const JsonObject& obj)
{
  double volume =
      get_value<MasterVolume, double>(this->_registry.get(), obj, e, "value")
          .value_or(100.0);
  init_component<MasterVolume>(
      this->_registry.get(), this->_event_manager.get(), e, volume);
}

void Sound::init_music_volume(Registry::Entity& e, const JsonObject& obj)
{
  double volume =
      get_value<MusicVolume, double>(this->_registry.get(), obj, e, "value")
          .value_or(100.0);
  init_component<MusicVolume>(
      this->_registry.get(), this->_event_manager.get(), e, volume);
}

void Sound::init_sfx_volume(Registry::Entity& e, const JsonObject& obj)
{
  double volume =
      get_value<SFXVolume, double>(this->_registry.get(), obj, e, "value")
          .value_or(100.0);
  init_component<SFXVolume>(
      this->_registry.get(), this->_event_manager.get(), e, volume);
}
