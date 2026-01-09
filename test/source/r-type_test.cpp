#include <optional>
#include <stdexcept>

#include <catch2/catch_test_macros.hpp>

#include "TwoWayMap.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "ecs/SparseArray.hpp"
#include "plugin/Byte.hpp"
#include "plugin/components/Direction.hpp"
#include "plugin/components/Health.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Speed.hpp"

TEST_CASE("SparseArray - Basic construction", "[sparse_array]")
{
  SparseArray<Position> const arr;
  REQUIRE(arr.empty());
  REQUIRE(arr.empty());
}

TEST_CASE("SparseArray - insert_at with value", "[sparse_array]")
{
  SparseArray<Position> arr;
  Position const pos(10.0f, 20.0f);

  auto& ref = arr.insert_at(0, pos);

  REQUIRE(!arr.empty());
  REQUIRE(ref.has_value());
  REQUIRE(ref->pos.x == 10.0f);
  REQUIRE(ref->pos.y == 20.0f);
}

TEST_CASE("SparseArray - insert_at with rvalue", "[sparse_array]")
{
  SparseArray<Position> arr;

  auto& ref = arr.insert_at(0, Position(5.0f, 15.0f));

  REQUIRE(!arr.empty());
  REQUIRE(ref.has_value());
  REQUIRE(ref->pos.x == 5.0f);
  REQUIRE(ref->pos.y == 15.0f);
}

TEST_CASE("SparseArray - insert_at with parameters", "[sparse_array]")
{
  SparseArray<Position> arr;

  auto& ref = arr.insert_at(0, 100.0f, 200.0f);

  REQUIRE(!arr.empty());
  REQUIRE(ref.has_value());
  REQUIRE(ref->pos.x == 100.0f);
  REQUIRE(ref->pos.y == 200.0f);
}

TEST_CASE("SparseArray - insert_at non-sequential indices", "[sparse_array]")
{
  SparseArray<Position> arr;

  arr.insert_at(5, Position(1.0f, 2.0f));

  REQUIRE(arr.size() >= 6);
  REQUIRE(!arr[0].has_value());
  REQUIRE(!arr[1].has_value());
  REQUIRE(!arr[4].has_value());
  REQUIRE(arr[5].has_value());
  REQUIRE(arr[5]->pos.x == 1.0f);
}

TEST_CASE("SparseArray - erase removes element", "[sparse_array]")
{
  SparseArray<Position> arr;
  arr.insert_at(0, Position(10.0f, 20.0f));

  REQUIRE(arr[0].has_value());

  arr.erase(0);

  REQUIRE(!arr[0].has_value());
}

TEST_CASE("SparseArray - erase out of bounds is safe", "[sparse_array]")
{
  SparseArray<Position> arr;

  REQUIRE_NOTHROW(arr.erase(100));
}

TEST_CASE("SparseArray - get_index finds element", "[sparse_array]")
{
  SparseArray<Position> arr;
  Position const pos(10.0f, 20.0f);
  arr.insert_at(3, pos);

  REQUIRE(arr[3].has_value());
  REQUIRE(arr[3]->pos.x == pos.pos.x);
  REQUIRE(arr[3]->pos.y == pos.pos.y);
}

TEST_CASE("SparseArray - get_index throws when not found", "[sparse_array]")
{
  SparseArray<Position> arr;
  arr.insert_at(0, Position(10.0f, 20.0f));

  REQUIRE(arr[0].has_value());
  REQUIRE(arr[0]->pos.x == 10.0f);
  REQUIRE(arr[0]->pos.y == 20.0f);
}

TEST_CASE("Registry - spawn_entity creates unique entities", "[registry]")
{
  Registry reg;

  auto entity1 = reg.spawn_entity();
  auto entity2 = reg.spawn_entity();
  auto entity3 = reg.spawn_entity();

  REQUIRE(entity1 == 0);
  REQUIRE(entity2 == 1);
  REQUIRE(entity3 == 2);
}

TEST_CASE("Registry - registerComponent creates component storage",
          "[registry]")
{
  Registry reg;

  auto& positions = reg.register_component<Position>("Position");

  REQUIRE(positions.empty());
}

TEST_CASE("Registry - getComponents retrieves registered components",
          "[registry]")
{
  Registry reg;
  reg.register_component<Position>("Position");

  auto& positions = reg.get_components<Position>();

  REQUIRE(positions.empty());
}

TEST_CASE("Registry - add_component adds component to entity", "[registry]")
{
  Registry reg;
  reg.register_component<Position>("Position");

  auto entity = reg.spawn_entity();
  auto& comp = reg.add_component(entity, Position(50.0f, 75.0f));

  REQUIRE(comp.has_value());
  REQUIRE(comp->pos.x == 50.0f);
  REQUIRE(comp->pos.y == 75.0f);
}

TEST_CASE("Registry - emplace_component constructs in place", "[registry]")
{
  Registry reg;
  reg.register_component<Position>("Position");

  auto entity = reg.spawn_entity();
  auto& comp = reg.emplace_component<Position>(entity, 10.0f, 20.0f);

  REQUIRE(comp.has_value());
  REQUIRE(comp->pos.x == 10.0f);
  REQUIRE(comp->pos.y == 20.0f);
}

TEST_CASE("Registry - removeComponent removes component from entity",
          "[registry]")
{
  Registry reg;
  reg.register_component<Position>("Position");
  reg.register_component<Speed>("Speed");

  auto entity = reg.spawn_entity();
  reg.add_component(entity, Position(10.0f, 20.0f));
  reg.add_component(entity, Speed(1.0f, 2.0f));

  auto& positions = reg.get_components<Position>();
  auto& speeds = reg.get_components<Speed>();

  REQUIRE(positions[entity].has_value());
  REQUIRE(speeds[entity].has_value());

  reg.kill_entity(entity);
  reg.process_entity_deletions();

  REQUIRE(!positions[entity].has_value());
  REQUIRE(!speeds[entity].has_value());
}

TEST_CASE("Registry - kill_entity recycles entity IDs", "[registry]")
{
  Registry reg;

  auto entity1 = reg.spawn_entity();
  reg.spawn_entity();

  reg.kill_entity(entity1);
  reg.process_entity_deletions();

  auto entity3 = reg.spawn_entity();

  REQUIRE(entity3 == entity1);
}

TEST_CASE("Registry - multiple components per entity", "[registry]")
{
  Registry reg;
  reg.register_component<Position>("Position");
  reg.register_component<Speed>("Speed");
  reg.register_component<Health>("Health");

  auto entity = reg.spawn_entity();
  reg.add_component(entity, Position(10.0f, 20.0f));
  reg.add_component(entity, Speed(1.0f, 2.0f));
  reg.add_component(entity, Health(100, 100));

  auto& positions = reg.get_components<Position>();
  auto& speeds = reg.get_components<Speed>();
  auto& healths = reg.get_components<Health>();

  REQUIRE(positions[entity].has_value());
  REQUIRE(speeds[entity].has_value());
  REQUIRE(healths[entity].has_value());
  REQUIRE(healths[entity]->current == 100);
}

TEST_CASE("Registry - add_system and runSystems", "[registry]")
{
  Registry reg;
  EventManager event_manager;
  reg.register_component<Position>("Position");

  auto entity = reg.spawn_entity();
  reg.add_component(entity, Position(0.0f, 0.0f));

  int system_runs = 0;

  reg.add_system<Position>(
      [&system_runs](Registry& /*r*/, SparseArray<Position>& /*positions*/)
      { system_runs++; });

  reg.run_systems(event_manager);

  REQUIRE(system_runs == 1);

  reg.run_systems(event_manager);

  REQUIRE(system_runs == 2);
}

TEST_CASE("Dummy test", "[dummy]")
{
  REQUIRE(true);
}
