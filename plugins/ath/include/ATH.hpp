#pragma once

#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "ecs/SparseArray.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Bar.hpp"
#include "plugin/components/Drawable.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/events/IoEvents.hpp"

class ATH : public APlugin
{
public:
  ATH(Registry& r,
      EventManager& em,
      EntityLoader& l,
      std::optional<JsonObject> const& config);

  void init_bar(Registry::Entity& e, const JsonObject& obj);

  void init_clickable(Registry::Entity const& e, JsonObject const& obj);
  void init_button(Registry::Entity const& e, JsonObject const& obj);
};
