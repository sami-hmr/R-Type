#pragma once

#include <concepts>

#include "Json/JsonParser.hpp"
#include "plugin/Byte.hpp"
class Registry;

template<typename T>
concept json_buildable = requires(Registry& r, JsonObject const& j) {
  { T(r, j) } -> std::same_as<T>;
};

template<typename T>
concept entity_convertible = requires(T& t) {
  { t.convert_entity() } -> std::same_as<ByteArray>;
};
