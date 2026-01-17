#pragma once

#include "BaseTypes.hpp"
#include "ByteParser/ByteParser.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct MasterVolume
{
  double value = 100.0;

  MasterVolume() = default;

  MasterVolume(double volume)
      : value(volume)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(MasterVolume,
                           ([](double volume) { return MasterVolume(volume); }),
                           parseByte<double>())

  DEFAULT_SERIALIZE(type_to_byte(this->value))

  CHANGE_ENTITY_DEFAULT

  HOOKABLE(MasterVolume, HOOK(value))
};

struct MusicVolume
{
  double value = 100.0;

  MusicVolume() = default;

  MusicVolume(double volume)
      : value(volume)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(MusicVolume,
                           ([](double volume) { return MusicVolume(volume); }),
                           parseByte<double>())

  DEFAULT_SERIALIZE(type_to_byte(this->value))

  CHANGE_ENTITY_DEFAULT

  HOOKABLE(MusicVolume, HOOK(value))
};

struct SFXVolume
{
  double value = 100.0;

  SFXVolume() = default;

  SFXVolume(double volume)
      : value(volume)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(SFXVolume,
                           ([](double volume) { return SFXVolume(volume); }),
                           parseByte<double>())

  DEFAULT_SERIALIZE(type_to_byte(this->value))

  CHANGE_ENTITY_DEFAULT

  HOOKABLE(SFXVolume, HOOK(value))
};
