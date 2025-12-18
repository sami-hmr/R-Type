#include <array>
#include <cstdint>
#include <functional>
#include <unordered_map>

#include "plugin/Byte.hpp"

#include <sys/types.h>

#include "Json/JsonParser.hpp"

ByteArray operator+(ByteArray first, ByteArray const& second)
{
  first.insert(first.end(), second.begin(), second.end());
  return first;
}

ByteArray& operator+=(ByteArray& first, ByteArray const& second)
{
  first.insert(first.end(), second.begin(), second.end());
  return first;
}

ByteArray string_to_byte(std::string const& str)
{
  return type_to_byte<u_int32_t>(str.size())
      + ByteArray(str.begin(), str.end());
}

static const std::array<std::function<ByteArray(JsonValue const&)>, 6>
    json_value_map = {
        [](JsonValue const& v) { return type_to_byte(std::get<int>(v.value)); },
        [](JsonValue const& v)
        { return type_to_byte(std::get<double>(v.value)); },
        [](JsonValue const& v)
        { return string_to_byte(std::get<std::string>(v.value)); },
        [](JsonValue const& v)
        { return type_to_byte(std::get<bool>(v.value)); },
        [](JsonValue const& v)
        { return json_object_to_byte(std::get<JsonObject>(v.value)); },
        [](JsonValue const& v)
        {
          return vector_to_byte(
              std::get<JsonArray>(v.value),
              std::function<ByteArray(JsonValue const& v)>(
                  [](JsonValue const& v) { return json_value_to_byte(v); }));
        }};

ByteArray json_value_to_byte(JsonValue const& v)
{
  return type_to_byte<std::uint8_t>(v.value.index())
      + json_value_map[v.value.index()](v);
}

ByteArray json_object_to_byte(JsonObject const& object)
{
  return map_to_byte(
      object,
      std::function<ByteArray(const std::string&)>(
          [](std::string const& s) { return string_to_byte(s); }),
      std::function<ByteArray(const JsonValue&)>(
          [](JsonValue const& s) { return json_value_to_byte(s); }));
}
