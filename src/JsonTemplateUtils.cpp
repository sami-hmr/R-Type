#include <iostream>
#include <variant>

#include "ecs/JsonTemplateUtils.hpp"

#include "Json/JsonParser.hpp"

static void replace_json_value(JsonValue& value,
                               JsonObject const& params,
                               JsonObject const& default_parameters)
{
  const auto visitor = Overloads {
      [](int) {},
      [](double) {},
      [params, default_parameters, &value](std::string const& str)
      {
        if (!str.starts_with('$')) {
          return;
        }
        std::string const& arg = str.substr(1);
        if (!params.contains(arg)) {
          if (default_parameters.contains(arg)) {
            value = default_parameters.at(arg);
          } else {
            std::cout << "warning, no default value for parameter " << arg
                      << "\n";
          }
          return;
        }
        value = params.at(arg);
      },
      [](bool) {},
      [params, default_parameters](JsonObject& obj)
      { replace_json_object(obj, params, default_parameters); },
      [params, default_parameters](JsonArray& array)
      { replace_json_array(array, params, default_parameters); },
  };

  std::visit(visitor, value.value);
}

void replace_json_array(JsonArray& arr,
                        JsonObject const& params,
                        JsonObject const& default_parameters)
{
  for (auto& value : arr) {
    replace_json_value(value, params, default_parameters);
  }
}

void replace_json_object(JsonObject& obj,
                         JsonObject const& params,
                         JsonObject const& default_parameters)
{
  for (auto& [_, value] : obj) {
    replace_json_value(value, params, default_parameters);
  }
}
