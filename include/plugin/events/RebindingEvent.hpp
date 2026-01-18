#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

#include "ByteParser/ByteParser.hpp"
#include "EventMacros.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/Entity.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/Hooks.hpp"
#include "plugin/events/IoEvents.hpp"

struct Rebind
{
  Ecs::Entity entity;
  std::uint16_t key_to_replace;
  std::uint16_t replacement_key;

  Rebind(Ecs::Entity entity,
         std::uint16_t key_to_replace,
         std::uint16_t replacement_key)
      : entity(entity)
      , key_to_replace(key_to_replace)
      , replacement_key(replacement_key)
  {
  }

  Rebind(Registry& r, JsonObject const& e, std::optional<Ecs::Entity> entity)
      : entity(get_value_copy<Ecs::Entity>(r, e, "entity", entity).value())
      , key_to_replace(
            static_cast<std::uint32_t>(KEY_MAPPING.at_first(
                get_value_copy<std::string>(r, e, "key_to_replace", entity)
                    .value()))
            << 8)
      , replacement_key(
            static_cast<std::uint32_t>(KEY_MAPPING.at_first(
                get_value_copy<std::string>(r, e, "replacement_key", entity)
                    .value()))
            << 8)
  {
  }

  CHANGE_ENTITY_DEFAULT

  DEFAULT_BYTE_CONSTRUCTOR(
      Rebind,  // NOLINT
      ([](Ecs::Entity entity,
          std::uint16_t key_to_replace,
          std::uint16_t replacement_key)
       { return Rebind(entity, key_to_replace, replacement_key); }),
      parseByte<Ecs::Entity>(),
      parseByte<std::uint16_t>(),
      parseByte<std::uint16_t>())

  DEFAULT_SERIALIZE(type_to_byte(entity),
                    type_to_byte(key_to_replace),
                    type_to_byte(replacement_key))
};

struct GenerateRebindingScene
{
  GenerateRebindingScene() = default;

  GenerateRebindingScene(std::size_t e,
                         std::string bg,
                         std::string btn,
                         std::string txt,
                         std::string lnk,
                         std::string back,
                         std::string base,
                         bool main)
      : entity(e)
      , background_template(std::move(bg))
      , button_template(std::move(btn))
      , text_template(std::move(txt))
      , link_template(std::move(lnk))
      , back_to_base_scene_template(std::move(back))
      , base_scene(std::move(base))
      , is_base_scene_main(main)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(GenerateRebindingScene,
                           (
                               [](std::size_t e,
                                  std::string const& bg,
                                  std::string const& btn,
                                  std::string const& txt,
                                  std::string const& lnk,
                                  std::string const& back,
                                  std::string const& base,
                                  bool main) {
                                 return GenerateRebindingScene(
                                     e, bg, btn, txt, lnk, back, base, main);
                               }),
                           parseByte<std::size_t>(),
                           parseByteString(),
                           parseByteString(),
                           parseByteString(),
                           parseByteString(),
                           parseByteString(),
                           parseByteString(),
                           parseByte<bool>())

  DEFAULT_SERIALIZE(type_to_byte(entity),
                    string_to_byte(this->background_template),
                    string_to_byte(this->button_template),
                    string_to_byte(this->text_template),
                    string_to_byte(this->link_template),
                    string_to_byte(this->back_to_base_scene_template),
                    string_to_byte(this->base_scene),
                    type_to_byte(is_base_scene_main))

  GenerateRebindingScene(Registry& r,
                         JsonObject const& e,
                         std::optional<Ecs::Entity> entity = std::nullopt)
      : entity(get_value_copy<Ecs::Entity>(r, e, "entity", entity).value_or(0))
      , background_template(
            get_value_copy<std::string>(r, e, "background_template", entity)
                .value_or(""))
      , button_template(
            get_value_copy<std::string>(r, e, "button_template", entity)
                .value_or(""))
      , text_template(get_value_copy<std::string>(r, e, "text_template", entity)
                          .value_or(""))
      , link_template(get_value_copy<std::string>(r, e, "link_template", entity)
                          .value_or(""))
      , card_template(get_value_copy<std::string>(r, e, "card_template", entity)
                          .value_or(""))
      , back_to_base_scene_template(
            get_value_copy<std::string>(r, e, "back_to_base_scene_template")
                .value_or(""))
      , base_scene(get_value_copy<std::string>(r, e, "base_scene", entity)
                       .value_or(""))
      , is_base_scene_main(
            get_value_copy<bool>(r, e, "is_base_scene_main", entity)
                .value_or(false))
  {
  }

  CHANGE_ENTITY_DEFAULT

  std::size_t entity = 0;
  std::string background_template;
  std::string button_template;
  std::string text_template;
  std::string link_template;
  std::string card_template;
  std::string back_to_base_scene_template;
  std::string base_scene;
  bool is_base_scene_main;
};

struct WatchRebind
{
  WatchRebind() = default;

  WatchRebind(Ecs::Entity entity, std::uint16_t key)
      : entity(entity)
      , key(key)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(WatchRebind,
                           ([](Ecs::Entity entity, std::uint16_t k)
                            { return WatchRebind(entity, k); }),
                           parseByte<Ecs::Entity>(),
                           parseByte<std::uint16_t>())

  DEFAULT_SERIALIZE(type_to_byte(entity), type_to_byte(key))

  WatchRebind(Registry& r,
              JsonObject const& e,
              std::optional<Ecs::Entity> entity)
      : entity(get_value_copy<int>(r, e, "entity", entity).value_or(0))
      , key(get_value_copy<int>(r, e, "key", entity).value_or(0))
  {
  }

  CHANGE_ENTITY(result.entity = map.at(entity);)

  Ecs::Entity entity;
  std::uint16_t key;
};

struct ExitRebind
{
  ExitRebind() = default;

  EMPTY_BYTE_CONSTRUCTOR(ExitRebind)
  DEFAULT_SERIALIZE(ByteArray {})

  CHANGE_ENTITY_DEFAULT

  ExitRebind(Registry& /*unused*/,
             JsonObject const& /*unused*/,
             std::optional<Ecs::Entity> /*unused*/)
  {
  }

  HOOKABLE(ExitRebind)
};
