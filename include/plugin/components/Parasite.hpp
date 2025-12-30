#pragma once

#include <optional>
#include <string>

#include <sys/types.h>

#include "BaseTypes.hpp"
#include "ByteParser/ByteParser.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/HookMacros.hpp"
#include "plugin/events/EventMacros.hpp"

struct Parasite
{
  std::optional<std::size_t> player_linked;
  std::string behaviour;  // attached / follow
  std::string effect;  // drain / attack
  Vector2D dflt_speed;

  Parasite() = default;

  Parasite(std::optional<std::size_t> player_linked,
           std::string behaviour,
           std::string effect,
           Vector2D dflt_speed)
      : player_linked(player_linked)
      , behaviour(std::move(behaviour))
      , effect(std::move(effect))
      , dflt_speed(dflt_speed)
  {
  }

  HOOKABLE(Parasite,
           HOOK(player_linked),
           HOOK(behaviour),
           HOOK(effect),
           HOOK(dflt_speed))

  CHANGE_ENTITY_DEFAULT

  DEFAULT_BYTE_CONSTRUCTOR(
      Parasite,
      ([](std::optional<std::size_t> player_linked,
          std::string behaviour,
          std::string effect,
          Vector2D dflt_speed)
       { return Parasite(player_linked, behaviour, effect, dflt_speed); }),
      parseByteOptional(parseByte<std::size_t>()),
      parseByteString(),
      parseByteString(),
      parseVector2D())

  DEFAULT_SERIALIZE(optional_to_byte<std::size_t>(
                        this->player_linked,
                        std::function<ByteArray(std::size_t const&)>(
                            [](std::size_t const& b)
                            { return type_to_byte(b); })),
                    string_to_byte(this->behaviour),
                    string_to_byte(this->effect),
                    vector2DToByte(this->dflt_speed))
};
