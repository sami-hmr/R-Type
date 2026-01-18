#pragma once

#include <string>
#include <variant>
#include <vector>

#include <sys/types.h>

#include "ByteParser/ByteParser.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Entity.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/LogMacros.hpp"

struct Item
{
  std::vector<JsonObject> on_use;

  Item() = default;

  Item(std::vector<JsonObject> const& on_use)
      : on_use(on_use)
  {
  }

  DEFAULT_SERIALIZE(vector_to_byte(
      on_use, SERIALIZE_FUNCTION<JsonObject>(json_object_to_byte)))

  Item(Registry& r, JsonObject const& e, Ecs::Entity entity)
  {
    auto const& on_use_array =
        get_value_copy<JsonArray>(r, e, "on_use", entity);
    if (!on_use_array) {
      LOGGER_EVTLESS(
          "item", LogLevel::WARNING, "on_use invalid parsing json array");
      return;
    }
    for (auto const& it : *on_use_array) {
      auto const* obj = std::get_if<JsonObject>(&it.value);
      if (obj == nullptr) {
        LOGGER_EVTLESS(
            "item", LogLevel::WARNING, "failed to parse json object");
        continue;
      }
      this->on_use.push_back(*obj);
    }
  }
};

inline Parser<Item> parse_byte_item()
{
  return apply(
      [](const std::vector<JsonObject>& to_use)
      {
        return Item(to_use);
      },
      parseByteArray(parseByteJsonObject()));
}
