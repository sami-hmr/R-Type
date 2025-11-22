#pragma once

#include "Json/JsonParser.hpp"
#include <iostream>
#include <stdexcept>
#include <variant>
#include "ecs/Registery.hpp"
#include "ecs/SparseArray.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "plugin/APlugin.hpp"

struct position {
    position(float x, float y): x(x), y(y) {}
    double x;
    double y;
};

class Moving: public APlugin {
    public:
    Moving(Registery &r, EntityLoader &l): APlugin(r, l, {
        COMP_INIT(position, init_pos)
    }) {
        this->registery_.get().registerComponent<position>();
        this->registery_.get().addSystem<position>([](Registery &, SparseArray<position>s) {
            for (auto &&[position] : Zipper(s)) {
                std::cout << position.x << "  " << position.y << std::endl;
            }
        });
    }
    private:

    void init_pos(Registery::Entity const entity, JsonVariant const &config) {
        try {
            JsonObject obj = std::get<JsonObject>(config);
            double x = std::get<double>(obj.at("x").value);
            double y = std::get<double>(obj.at("y").value);
            this->registery_.get().emplace_component<position>(entity, x, y);
        } catch (std::bad_variant_access const &) {
            std::cerr << "Error loading position component: unexpected value type" << std::endl;
        } catch (std::out_of_range const &) {
            std::cerr << "Error loading position component: missing value in JsonObject" << std::endl;
        }
    }

    const std::vector<std::string> depends_on_ = {};
};
