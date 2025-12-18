#include "plugin/components/Clickable.hpp"
#include "ATH.hpp"
#include "ecs/InitComponent.hpp"
#include "ecs/zipper/Zipper.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/components/Collidable.hpp"
#include "libs/Rect.hpp"


void ATH::init_clickable(Registry::Entity const& e, JsonObject const& obj)
{
    std::vector<std::pair<std::string, JsonObject>> emits;

    if (obj.contains("to_emit")) {
        try {
            JsonArray emits_array = std::get<JsonArray>(obj.at("to_emit").value);
            for (auto const &emit_value : emits_array) {
                try {
                    JsonObject emit_obj = std::get<JsonObject>(emit_value.value);
                    for (auto const &[event_name, event_data_value] : emit_obj) {
                        try {
                            JsonObject event_data = std::get<JsonObject>(event_data_value.value);
                            emits.emplace_back(event_name, event_data);
                        } catch (std::bad_variant_access const&) {
                            std::cerr << "Error parsing clickable emit: event data is not a JsonObject" << '\n';
                        }
                    }
                } catch (std::bad_variant_access const&) {
                    std::cerr << "Error parsing clickable emit: invalid format" << '\n';
                }
            }
        } catch (std::bad_variant_access const&) {
            std::cerr << "Error parsing clickable component: 'to_emit' is not a JsonArray" << '\n';
        }
    }
    init_component<Clickable>(this->_registry.get(), e, emits);
}
