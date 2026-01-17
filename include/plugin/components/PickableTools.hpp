#include <optional>

#include "ByteParser/ByteParser.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/HookMacros.hpp"
#include "plugin/events/EventMacros.hpp"

struct PickableTool
{
  std::optional<JsonObject> on_consumption;
  std::optional<JsonObject> on_throw;
  std::string name;
  bool consumable;
  bool throwable;

  PickableTool(std::optional<JsonObject> on_consumption,
               std::optional<JsonObject> on_throw,
               std::string name,
               bool consumable,
               bool throwable)
      : on_consumption(std::move(on_consumption))
      , on_throw(std::move(on_throw))
      , name(std::move(name))
      , consumable(consumable)
      , throwable(throwable)
  {
  }

  PickableTool(std::string name)
      : on_consumption(std::nullopt)
      , on_throw(std::nullopt)
      , name(std::move(name))
      , consumable(false)
      , throwable(false)
  {
  }

  CHANGE_ENTITY_DEFAULT

  HOOKABLE(PickableTool,
           HOOK(on_consumption),
           HOOK(on_throw),
           HOOK(name),
           HOOK(consumable),
           HOOK(throwable))

  DEFAULT_BYTE_CONSTRUCTOR(PickableTool,
                           (
                               [](std::optional<JsonObject> on_consumption,
                                  std::optional<JsonObject> on_throw,
                                  std::string name,
                                  bool consumable,
                                  bool throwable)
                               {
                                 return PickableTool(std::move(on_consumption),
                                                     std::move(on_throw),
                                                     std::move(name),
                                                     consumable,
                                                     throwable);
                               }),
                           parseByteOptional(parseByteJsonObject()),
                           parseByteOptional(parseByteJsonObject()),
                           parseByteString(),
                           parseByte<bool>(),
                           parseByte<bool>())

  DEFAULT_SERIALIZE(
      optional_to_byte<JsonObject>(on_consumption,
                                   std::function<ByteArray(JsonObject)>(
                                       [](JsonObject const& o)
                                       { return json_object_to_byte(o); })),

      optional_to_byte<JsonObject>(on_throw,
                                   std::function<ByteArray(JsonObject)>(
                                       [](JsonObject const& o)
                                       { return json_object_to_byte(o); })),
      string_to_byte(this->name),
      type_to_byte(this->consumable),
      type_to_byte(this->throwable))
};
