/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** LogEvent
*/

#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <utility>

#include "ByteParser/ByteParser.hpp"
#include "ParserUtils.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"

enum class LogLevel : std::uint8_t
{
  DEBUG,
  INFO,
  WARNING,
  ERROR
};

struct LogEvent
{
    LogEvent(std::string n, LogLevel l, std::string m)
      : name(std::move(n))
      , level(l)
      , message(std::move(m)) {}

    DEFAULT_BYTE_CONSTRUCTOR(LogEvent,
                           ([](std::string const& n, LogLevel l, std::string const& m)
                            { return (LogEvent) {n, l, m}; }),
                           parseByteString(),
                           parseByte<LogLevel>(),
                           parseByteString())

    DEFAULT_SERIALIZE(string_to_byte(this->name), type_to_byte(this->level), string_to_byte(this->message))

    std::string name;
    LogLevel level;
    std::string message;
};