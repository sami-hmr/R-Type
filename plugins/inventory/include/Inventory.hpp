#pragma once

#include <map>
#include "plugin/APlugin.hpp"
#include "plugin/components/Item.hpp"

    #define MAX_ITEMS 64

class Inventory : public APlugin
{
    public:
    Inventory(Registry& r, EventManager &em, EntityLoader& l);
    ~Inventory() override = default;
    EntityLoader &entity_loader;

    private:
        std::map<Item, std::size_t> _inventory;
        std::size_t _max_items = MAX_ITEMS;

        template <typename EventType>
        void use_item(const Item& item, std::size_t nb, bool usable);
        void add_item(Item &item, std::size_t nb);
        void throw_item(const Item &item, std::size_t nb);
        void delete_item(const Item &item, std::size_t nb);
        void consume_item(const Item &item, std::size_t nb);
        
};
