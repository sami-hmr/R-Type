#pragma once

#include <cstddef>

#include "ParserUtils.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct Follower
{
  Follower()
      : target(0)
      , lost_target(true)
  {
  }

  Follower(std::size_t target, bool lost_target)
      : target(target)
      , lost_target(lost_target)
  {
  }

  Follower(std::size_t target)
      : target(target)
      , lost_target(false)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(Follower,
                           ([](std::size_t target, bool lost_target)
                            { return (Follower) {target, lost_target}; }),
                           parseByte<std::size_t>(),
                           parseByte<bool>())
  DEFAULT_SERIALIZE(type_to_byte(this->target), type_to_byte(this->lost_target))

  HOOKABLE(Follower, HOOK(target), HOOK(lost_target))

  std::size_t target;
  bool lost_target;
};
