#include "libs/Vector2D.hpp"

#include "Json/JsonParser.hpp"
#include "plugin/Hooks.hpp"

Vector2D::Vector2D(JsonVariant const& variant,
                   std::string const& x,
                   std::string const& y)
    : Vector2D(std::get<JsonObject>(variant), x, y)
{
}

Vector2D::Vector2D(JsonObject const& obj,
                   std::string const& x,
                   std::string const& y)
{
  try {
    if (!is_hook(obj, x)) {
      this->x = std::get<double>(obj.at(x).value);
    }
    if (!is_hook(obj, y)) {
      this->y = std::get<double>(obj.at(y).value);
    }
  } catch (std::bad_variant_access const&) {
    std::string x_percentage = std::get<std::string>(obj.at(x).value);
    std::string y_percentage = std::get<std::string>(obj.at(y).value);

    if (!x_percentage.empty() && x_percentage.back() == '%') {
      x_percentage.pop_back();
    }
    if (!y_percentage.empty() && y_percentage.back() == '%') {
      y_percentage.pop_back();
    }

    try {
      this->x = std::stod(x_percentage) / 100.0;
      this->y = std::stod(y_percentage) / 100.0;
    } catch (std::invalid_argument const&) {
      std::cerr << "Error parsing Vector2D: unexpected value type\n";
      this->x = 0.0;
      this->y = 0.0;
    }
  } catch (std::out_of_range const&) {
    std::cerr << "Error parsing Vector2D: missing " << x << " or " << y
              << " in JsonObject\n";
    this->x = 0.0;
    this->y = 0.0;
  }
}
