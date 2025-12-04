#include "libs/Vector2D.hpp"

Vector2D::Vector2D(JsonVariant const& variant)
{
    try {
        JsonObject obj = std::get<JsonObject>(variant);
        this->x = std::get<double>(obj.at("x").value);
        this->y = std::get<double>(obj.at("y").value);
    } catch (std::bad_variant_access const&) {
        std::cerr << "Error parsing Vector2D: unexpected value type\n";
        this->x = 0.0;
        this->y = 0.0;
    } catch (std::out_of_range const&) {
        std::cerr << "Error parsing Vector2D: missing x or y in JsonObject\n";
        this->x = 0.0;
        this->y = 0.0;
    }
}
