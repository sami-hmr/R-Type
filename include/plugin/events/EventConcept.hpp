#pragma once

#include <concepts>

#include "Json/JsonParser.hpp"
class Registery;

template<typename T>
concept EventIsJsonBuilable = requires(Registery& r, JsonObject const& j) {
  { T(r, j) } -> std::same_as<T>;
};
