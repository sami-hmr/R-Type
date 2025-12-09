#pragma once

#include <concepts>

#include "Json/JsonParser.hpp"
class Registry;

template<typename T>
concept EventIsJsonBuilable = requires(Registry& r, JsonObject const& j) {
  { T(r, j) } -> std::same_as<T>;
};
