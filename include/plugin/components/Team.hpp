#pragma once

#include <string>

#include "ByteParser/ByteParser.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct Team
{
  Team() = default;

  Team(std::string name)
      : name(std::move(name))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      Team,
      ([](std::vector<char> name_vec)
       { return Team(std::string(name_vec.begin(), name_vec.end())); }),
      parseByteArray(parseAnyChar()))
  DEFAULT_SERIALIZE(string_to_byte(this->name))

  CHANGE_ENTITY_DEFAULT

  std::string name;

  HOOKABLE(Team, HOOK(name))
};
