#pragma once

#include <optional>
#include <string>

#include <sys/types.h>

#include "BaseTypes.hpp"
#include "ByteParser/ByteParser.hpp"
#include "ecs/Registry.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/HookMacros.hpp"
#include "plugin/events/EventMacros.hpp"

struct Parasite
{
  std::optional<std::size_t> entity_id;
  std::string behavior;

  Parasite() = default;

  Parasite(std::optional<std::size_t> entity_id, std::string behavior)
      : entity_id(entity_id)
      , behavior(std::move(behavior))
  {
  }

  Parasite(std::string behavior)
      : entity_id(std::nullopt)
      , behavior(std::move(behavior))
  {
  }

  HOOKABLE(Parasite, HOOK(entity_id), HOOK(behavior))

  CHANGE_ENTITY(if (entity_id.has_value()) {
    result.entity_id.emplace(map.at(entity_id.value()));
  })

  DEFAULT_BYTE_CONSTRUCTOR(Parasite,
                           ([](std::optional<std::size_t> entity_id,
                               std::string const& behavior)
                            { return Parasite(entity_id, behavior); }),
                           parseByteOptional(parseByte<std::size_t>()),
                           parseByteString())

  DEFAULT_SERIALIZE(optional_to_byte<std::size_t>(
                        this->entity_id,
                        std::function<ByteArray(std::size_t const&)>(
                            [](std::size_t const& b)
                            { return type_to_byte(b); })),
                    string_to_byte(this->behavior))
};
