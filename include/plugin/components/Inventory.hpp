
#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>

#include "ByteParser/ByteParser.hpp"
#include "ParserUtils.hpp"
#include "plugin/Byte.hpp"
#include "plugin/components/Item.hpp"
#include "plugin/events/EventMacros.hpp"

struct Inventory
{
  struct ItemSlot
  {
    ItemSlot(std::string s,
             std::size_t nb,
             Item item,
             std::string artefact_template)
        : item_name(std::move(s))
        , nb(nb)
        , item(std::move(item))
        , artefact_template(std::move(artefact_template))
    {
    }

    std::string item_name;
    std::size_t nb;
    Item item;
    std::string artefact_template;

    DEFAULT_BYTE_CONSTRUCTOR(
        ItemSlot,
        [](std::string const& s,
           std::size_t nb,
           Item const& item,
           std::string artefact_template)
        { return ItemSlot(s, nb, item, std::move(artefact_template)); },
        parseByteString(),
        parseByte<std::size_t>(),
        parse_byte_item(),
        parseByteString())

    DEFAULT_SERIALIZE(string_to_byte(item_name),
                      type_to_byte(nb),
                      item.to_bytes(),
                      string_to_byte(artefact_template))
  };

  std::vector<ItemSlot> inventory;
  std::size_t max_items;

  CHANGE_ENTITY_DEFAULT

  HOOKABLE(Inventory, HOOK(max_items))

  Inventory(std::vector<ItemSlot> const& inventory, std::size_t max_items)
      : inventory(inventory)
      , max_items(max_items)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      Inventory,
      ([](std::vector<ItemSlot> const& tmp, std::size_t max_items)
       { return Inventory(tmp, max_items); }),
      parseByteArray(apply([](std::string const& s,
                              std::size_t nb,
                              Item const& i,
                              std::string const& a)
                           { return ItemSlot(s, nb, i, a); },
                           parseByteString(),
                           parseByte<std::size_t>(),
                           parse_byte_item(),
                           parseByteString())),
      parseByte<std::size_t>())

  DEFAULT_SERIALIZE(
      vector_to_byte(this->inventory,
                     SERIALIZE_FUNCTION<ItemSlot>([](ItemSlot const& i)
                                                  { return i.to_bytes(); })),
      type_to_byte(this->max_items))
};

struct Pickable
{
  Pickable() = default;

  Pickable(std::string item_name,
           std::string artefact_template,
           JsonObject item)
      : item_name(std::move(item_name))
      , artefact_template(std::move(artefact_template))
      , item(std::move(item))
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      Pickable,
      ([](std::string const& n, std::string const& a, JsonObject const& i)
       { return Pickable(n, a, i); }),
      parseByteString(),
      parseByteString(),
      parseByteJsonObject())

  DEFAULT_SERIALIZE(string_to_byte(this->item_name),
                    string_to_byte(artefact_template),
                    json_object_to_byte(this->item))

  CHANGE_ENTITY_DEFAULT

  HOOKABLE(Pickable, HOOK(item_name), HOOK(artefact_template), HOOK(item))

  std::string item_name;
  std::string artefact_template;
  JsonObject item;
};
