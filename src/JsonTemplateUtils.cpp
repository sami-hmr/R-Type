#include <iostream>
#include <variant>
#include "ecs/JsonTemplateUtils.hpp"
#include "Json/JsonParser.hpp"

static void replace_json_value(JsonValue& value, JsonObject const& params)
{
  const auto visitor = Overloads {
      [](int) {},
      [](double) {},
      [params, &value](std::string const& str)
      {
        if (!str.starts_with('$')) {
          return;
        }
        std::string const& arg = str.substr(1);
        if (!params.contains(arg)) {
          return;
        }
        value = params.at(arg);
      },
      [](bool) {},
      [params](JsonObject& obj) { replace_json_object(obj, params); },
      [params](JsonArray& array) { replace_json_array(array, params); },
  };

  std::visit(visitor, value.value);
}

void replace_json_array(JsonArray& arr, JsonObject const& params)
{
  for (auto& value : arr) {
    replace_json_value(value, params);
  }
}

void replace_json_object(JsonObject& obj, JsonObject const& params)
{
  for (auto& [_, value] : obj) {
    replace_json_value(value, params);
  }
}
