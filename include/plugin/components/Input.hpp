#pragma once

#include <string>
#include <utility>

#include "ByteParser/ByteParser.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"

struct Input
{
  Input() = default;

  Input(bool enabled, std::string buffer = "")
      : enabled(enabled)
      , buffer(std::move(buffer))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      Input,
      ([](bool e, std::vector<char> b)
       { return Input(e, std::string(b.begin(), b.end())); }),
      parseByte<bool>(),
      parseByteArray(parseAnyChar()))

  DEFAULT_SERIALIZE(type_to_byte(this->enabled), string_to_byte(this->buffer))

  HOOKABLE(Input, HOOK(enabled), HOOK(buffer))

  bool enabled = false;
  std::string buffer;
};
