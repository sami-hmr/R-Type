
#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#include "ByteParser/ByteParser.hpp"
#include "ecs/Registry.hpp"
#include "plugin/Byte.hpp"
#include "plugin/components/Item.hpp"

struct Inventory
{
  using Object = std::pair<std::string, JsonObject>;
  using Slots = std::vector<std::pair<Item, std::size_t>>;
  Slots slots;
  std::string all_content;
  Registry::Entity owner;
  std::size_t max_items;

  CHANGE_ENTITY(result.owner, map.at(owner))

  HOOKABLE(Inventory,
           {"slot_1",
            [](Component& self) -> std::any
            {
              if (self.slots.size() < 1) {
                return std::reference_wrapper("");
              }
              return std::reference_wrapper(self.slots[0].first.object.first);
            }},
           {"slot_2",
            [](Component& self) -> std::any
            {
              if (self.slots.size() < 2) {
                return std::reference_wrapper("");
              }
              return std::reference_wrapper(self.slots[1].first.object.first);
            }},
           {"slot_3",
            [](Component& self) -> std::any
            {
              if (self.slots.size() < 3) {
                return std::reference_wrapper("");
              }
              return std::reference_wrapper(self.slots[2].first.object.first);
            }})

  Inventory(Slots slots,
            std::string all_content,
            Registry::Entity owner,
            std::size_t max_items)
      : slots(std::move(slots))
      , all_content(std::move(all_content))
      , owner(owner)
      , max_items(max_items)
  {
  }

  DEFAULT_BYTE_CONSTRUCTOR(
      Inventory,
      (
          [](Slots slots,
             std::string all_content,
             Registry::Entity owner,
             std::size_t max_items)
          {
            return Inventory(
                std::move(slots), std::move(all_content), owner, max_items);
          }),
      parseByteArray(parseBytePair(parseByteItem(), parseByte<std::size_t>())),
      parseByteString(),
      parseByte<Registry::Entity>(),
      parseByte<std::size_t>())

  DEFAULT_SERIALIZE(
      vector_to_byte(
          this->slots,
          std::function<ByteArray(const std::pair<Item, std::size_t>&)>(
              [](const std::pair<Item, std::size_t>& p)
              {
                return byte_array_join(item_to_byte(p.first),
                                       type_to_byte(p.second));
              })),
      string_to_byte(this->all_content),
      type_to_byte(this->owner),
      type_to_byte(this->max_items))
};
