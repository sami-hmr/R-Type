#include <catch2/catch_test_macros.hpp>

#include "ecs/Registery.hpp"
#include "ecs/SparseArray.hpp"

// Test components
struct Position
{
  float x;
  float y;

  bool operator==(const Position& other) const
  {
    return x == other.x && y == other.y;
  }
};

struct Velocity
{
  float dx;
  float dy;
};

struct Health
{
  int hp;
  static const int DefaultHp = 100;

  Health(int hp = DefaultHp)
      : hp(hp)
  {
  }
};

// ==================== SparseArray Tests ====================

TEST_CASE("SparseArray - Basic construction", "[sparse_array]")
{
  SparseArray<Position> arr;
  REQUIRE(arr.size() == 0);
  REQUIRE(arr.empty());
}

TEST_CASE("SparseArray - insert_at with value", "[sparse_array]")
{
  SparseArray<Position> arr;
  Position pos(10.0f, 20.0f);

  auto& ref = arr.insert_at(0, pos);

  REQUIRE(arr.size() >= 1);
  REQUIRE(ref.has_value());
  REQUIRE(ref->x == 10.0f);
  REQUIRE(ref->y == 20.0f);
}

TEST_CASE("SparseArray - insert_at with rvalue", "[sparse_array]")
{
  SparseArray<Position> arr;

  auto& ref = arr.insert_at(0, Position(5.0f, 15.0f));

  REQUIRE(arr.size() >= 1);
  REQUIRE(ref.has_value());
  REQUIRE(ref->x == 5.0f);
  REQUIRE(ref->y == 15.0f);
}

TEST_CASE("SparseArray - insert_at with parameters", "[sparse_array]")
{
  SparseArray<Position> arr;

  auto& ref = arr.insert_at(0, 100.0f, 200.0f);

  REQUIRE(arr.size() >= 1);
  REQUIRE(ref.has_value());
  REQUIRE(ref->x == 100.0f);
  REQUIRE(ref->y == 200.0f);
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
  REQUIRE(arr[5]->x == 1.0f);
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
  Position pos(10.0f, 20.0f);
  arr.insert_at(3, pos);

  auto index = arr.get_index(arr[3]);

  REQUIRE(index == 3);
}

TEST_CASE("SparseArray - get_index throws when not found", "[sparse_array]")
{
  SparseArray<Position> arr;
  arr.insert_at(0, Position(10.0f, 20.0f));

  std::optional<Position> nonExistent = Position(99.0f, 99.0f);

  REQUIRE_THROWS_AS(arr.get_index(nonExistent), std::out_of_range);
}

// ==================== Registery Tests ====================

TEST_CASE("Registery - spawn_entity creates unique entities", "[registery]")
{
  Registery reg;

  auto entity1 = reg.spawn_entity();
  auto entity2 = reg.spawn_entity();
  auto entity3 = reg.spawn_entity();

  REQUIRE(entity1 == 0);
  REQUIRE(entity2 == 1);
  REQUIRE(entity3 == 2);
}

TEST_CASE("Registery - registerComponent creates component storage",
          "[registery]")
{
  Registery reg;

  auto& positions = reg.registerComponent<Position>();

  REQUIRE(positions.size() == 0);
}

TEST_CASE("Registery - getComponents retrieves registered components",
          "[registery]")
{
  Registery reg;
  reg.registerComponent<Position>();

  auto& positions = reg.getComponents<Position>();

  REQUIRE(positions.size() == 0);
}

TEST_CASE("Registery - add_component adds component to entity", "[registery]")
{
  Registery reg;
  reg.registerComponent<Position>();

  auto entity = reg.spawn_entity();
  auto& comp = reg.add_component(entity, Position(50.0f, 75.0f));

  REQUIRE(comp.has_value());
  REQUIRE(comp->x == 50.0f);
  REQUIRE(comp->y == 75.0f);
}

TEST_CASE("Registery - emplace_component constructs in place", "[registery]")
{
  Registery reg;
  reg.registerComponent<Position>();

  auto entity = reg.spawn_entity();
  auto& comp = reg.emplace_component<Position>(entity, 10.0f, 20.0f);

  REQUIRE(comp.has_value());
  REQUIRE(comp->x == 10.0f);
  REQUIRE(comp->y == 20.0f);
}

TEST_CASE("Registery - removeComponent removes component from entity",
          "[registery]")
{
  Registery reg;
  reg.registerComponent<Position>();

  auto entity = reg.spawn_entity();
  reg.add_component(entity, Position(10.0f, 20.0f));

  auto& positions = reg.getComponents<Position>();
  REQUIRE(positions[entity].has_value());

  reg.removeComponent<Position>(entity);

  REQUIRE(!positions[entity].has_value());
}

TEST_CASE("Registery - kill_entity removes all components", "[registery]")
{
  Registery reg;
  reg.registerComponent<Position>();
  reg.registerComponent<Velocity>();

  auto entity = reg.spawn_entity();
  reg.add_component(entity, Position(10.0f, 20.0f));
  reg.add_component(entity, Velocity(1.0f, 2.0f));

  auto& positions = reg.getComponents<Position>();
  auto& velocities = reg.getComponents<Velocity>();

  REQUIRE(positions[entity].has_value());
  REQUIRE(velocities[entity].has_value());

  reg.kill_entity(entity);

  REQUIRE(!positions[entity].has_value());
  REQUIRE(!velocities[entity].has_value());
}

TEST_CASE("Registery - kill_entity recycles entity IDs", "[registery]")
{
  Registery reg;

  auto entity1 = reg.spawn_entity();
  auto entity2 = reg.spawn_entity();

  reg.kill_entity(entity1);

  auto entity3 = reg.spawn_entity();

  REQUIRE(entity3 == entity1);
}

TEST_CASE("Registery - multiple components per entity", "[registery]")
{
  Registery reg;
  reg.registerComponent<Position>();
  reg.registerComponent<Velocity>();
  reg.registerComponent<Health>();

  auto entity = reg.spawn_entity();
  reg.add_component(entity, Position(10.0f, 20.0f));
  reg.add_component(entity, Velocity(1.0f, 2.0f));
  reg.add_component(entity, Health(100));

  auto& positions = reg.getComponents<Position>();
  auto& velocities = reg.getComponents<Velocity>();
  auto& healths = reg.getComponents<Health>();

  REQUIRE(positions[entity].has_value());
  REQUIRE(velocities[entity].has_value());
  REQUIRE(healths[entity].has_value());
  REQUIRE(healths[entity]->hp == 100);
}

TEST_CASE("Registery - add_system and runSystems", "[registery]")
{
  Registery reg;
  reg.registerComponent<Position>();

  auto entity = reg.spawn_entity();
  reg.add_component(entity, Position(0.0f, 0.0f));

  int systemRuns = 0;

  reg.add_system<Position>(
      [&systemRuns](Registery& r, SparseArray<Position>& positions)
      { systemRuns++; });

  reg.runSystems();

  REQUIRE(systemRuns == 1);

  reg.runSystems();

  REQUIRE(systemRuns == 2);
}

TEST_CASE("Dummy test", "[dummy]")
{
  REQUIRE(true);
}
