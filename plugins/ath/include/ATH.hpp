#pragma once

#include "Json/JsonParser.hpp"
#include "ecs/Registry.hpp"
#include "ecs/SparseArray.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"
#include "plugin/components/Bar.hpp"
#include "plugin/components/Drawable.hpp"
#include "plugin/components/Position.hpp"

class ATH : public APlugin
{
public:
  ATH(Registry& r, EntityLoader& l, std::optional<JsonObject> const& config);

  void init_bar(Registry::Entity& e, const JsonObject& obj);
  void bar_system(Registry& registry,
                  const SparseArray<Scene>& scenes,
                  const SparseArray<Drawable>& drawables,
                  const SparseArray<Position>& positions,
                  SparseArray<Bar>& bars);

  void init_clickable(Registry::Entity const& e, JsonObject const& obj);
};
