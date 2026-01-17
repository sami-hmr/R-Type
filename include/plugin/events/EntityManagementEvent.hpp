#pragma once

#include <utility>
#include <vector>

#include "ByteParser/ByteParser.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/EventMacros.hpp"

struct LoadEntityTemplate
{
  using Additional = std::vector<std::pair<std::string, ByteArray>>;

  LoadEntityTemplate(std::string templ, Additional aditionals)
      : template_name(std::move(templ))
      , aditionals(std::move(aditionals))
  {
  }

  CHANGE_ENTITY_DEFAULT

  DEFAULT_BYTE_CONSTRUCTOR(
      LoadEntityTemplate,
      ([](std::string o, Additional const& a)
       { return LoadEntityTemplate(o, a); }),
      parseByteString(),
      parseByteArray(parseBytePair(parseByteString(),
                                   parseByteArray(parseByte<Byte>()))))

  DEFAULT_SERIALIZE(
      string_to_byte(template_name),
      vector_to_byte(
          aditionals,
          std::function<ByteArray(const std::pair<std::string, ByteArray>&)>(
              [](const std::pair<std::string, ByteArray>& pair)
              {
                return pair_to_byte(
                    pair,
                    std::function<ByteArray(const std::string&)>(
                        [](const std::string& s) { return string_to_byte(s); }),
                    std::function<ByteArray(const ByteArray&)>(
                        [](ByteArray const& b)
                        {
                          return vector_to_byte(
                              b,
                              std::function<ByteArray(const Byte&)>(
                                  [](Byte const& b)
                                  { return type_to_byte<Byte>(b); }));
                        }));
              })))

  LoadEntityTemplate(Registry& r, JsonObject const& e, std::optional<Ecs::Entity> entity)
      : template_name(get_value_copy<std::string>(r, e, "template", entity).value())
  {
  }

  std::string template_name;
  Additional aditionals;
};

struct DeleteEntity
{
  DeleteEntity() = default;

  DeleteEntity(Ecs::Entity e)
      : entity(e)
  {
  }

  CHANGE_ENTITY(result.entity = map.at(entity))

  DEFAULT_BYTE_CONSTRUCTOR(DeleteEntity,
                           ([](Ecs::Entity e) { return DeleteEntity(e); }),
                           parseByte<Ecs::Entity>())

  DEFAULT_SERIALIZE(type_to_byte(entity))

  DeleteEntity(Registry& r, JsonObject const& conf, std::optional<Ecs::Entity> entity)
      : entity(*get_value_copy<int>(r, conf, "entity", entity))
  {
  }

  Ecs::Entity entity;
};

struct DeleteClientEntity
{
  DeleteClientEntity() = default;

  DeleteClientEntity(Ecs::Entity e)
      : entity(e)
  {
  }

  CHANGE_ENTITY(result.entity = map.at(entity))

  DEFAULT_BYTE_CONSTRUCTOR(DeleteClientEntity,
                           ([](Ecs::Entity e)
                            { return DeleteClientEntity(e); }),
                           parseByte<Ecs::Entity>())

  DEFAULT_SERIALIZE(type_to_byte(entity))

  DeleteClientEntity(Registry& r, JsonObject const& conf, std::optional<Ecs::Entity> entity)
      : entity(*get_value_copy<int>(r, conf, "entity", entity))
  {
  }

  Ecs::Entity entity;
};
