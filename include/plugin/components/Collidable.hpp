#pragma once

#include <string>
#include <vector>

#include "BaseTypes.hpp"
#include "ByteParser/ByteParser.hpp"
#include "plugin/Byte.hpp"

struct Collidable
{
  Collidable() = default;

  Collidable(double width, double height)
      : width(width)
      , height(height)
  {
  }

  Collidable(double width,
             double height,
             std::vector<std::string> const& exclude)
      : width(width)
      , height(height)
      , exclude_entities(exclude)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      Collidable,
      (
          [](double x, double y, std::vector<std::vector<char>> const& excludes)
          {
            std::vector<std::string> r(excludes.size());
            for (auto const& it : excludes) {
              r.emplace_back(it.begin(), it.end());
            }
            return Collidable(x, y, r);
          }),
      parseByte<double>(),
      parseByte<double>(),
      parseByteArray(parseByteArray(parseAnyChar())))
  DEFAULT_SERIALIZE(type_to_byte(this->width),
                    type_to_byte(this->height),
                    vector_to_byte<std::string>(this->exclude_entities,
                                                string_to_byte))

  double width = 0.0;
  double height = 0.0;
  std::vector<std::string> exclude_entities;
};
