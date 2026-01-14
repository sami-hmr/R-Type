
#include <string>
#include <vector>
#include "ecs/Registry.hpp"

struct Inventory {
  std::vector<std::string> inventory;
  std::string full_inventory;
  Registry::Entity owner;

  CHANGE_ENTITY(result.owner, map.at(owner))

  HOOKABLE(Inventory, { \
    "slot_1", [](Component& self) -> std::any \
    { return std::reference_wrapper(self.inventory[0]); } \
  })
};