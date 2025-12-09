#include <optional>
#include <stdexcept>

#include <catch2/catch_test_macros.hpp>

#include "ecs/Registry.hpp"
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
  static const int default_hp = 100;

  Health(int hp = default_hp)
      : hp(hp)
  {
  }
};

// ==================== SparseArray Tests ====================

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
  REQUIRE(ref->x == 10.0f);
  REQUIRE(ref->y == 20.0f);
}

TEST_CASE("SparseArray - insert_at with rvalue", "[sparse_array]")
{
  SparseArray<Position> arr;

  auto& ref = arr.insert_at(0, Position(5.0f, 15.0f));

  REQUIRE(!arr.empty());
  REQUIRE(ref.has_value());
  REQUIRE(ref->x == 5.0f);
  REQUIRE(ref->y == 15.0f);
}

TEST_CASE("SparseArray - insert_at with parameters", "[sparse_array]")
{
  SparseArray<Position> arr;

  auto& ref = arr.insert_at(0, 100.0f, 200.0f);

  REQUIRE(!arr.empty());
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
  Position const pos(10.0f, 20.0f);
  arr.insert_at(3, pos);

  auto index = arr.get_index(arr[3]);

  REQUIRE(index == 3);
}

TEST_CASE("SparseArray - get_index throws when not found", "[sparse_array]")
{
  SparseArray<Position> arr;
  arr.insert_at(0, Position(10.0f, 20.0f));

  std::optional<Position> const non_existent = Position(99.0f, 99.0f);

  REQUIRE_THROWS_AS(arr.get_index(non_existent), std::out_of_range);
}

// ==================== Registry Tests ====================

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

  auto& positions = reg.register_component<Position>();

  REQUIRE(positions.empty());
}

TEST_CASE("Registry - getComponents retrieves registered components",
          "[registry]")
{
  Registry reg;
  reg.register_component<Position>();

  auto& positions = reg.get_components<Position>();

  REQUIRE(positions.empty());
}

TEST_CASE("Registry - add_component adds component to entity", "[registry]")
{
  Registry reg;
  reg.register_component<Position>();

  auto entity = reg.spawn_entity();
  auto& comp = reg.add_component(entity, Position(50.0f, 75.0f));

  REQUIRE(comp.has_value());
  REQUIRE(comp->x == 50.0f);
  REQUIRE(comp->y == 75.0f);
}

TEST_CASE("Registry - emplace_component constructs in place", "[registry]")
{
  Registry reg;
  reg.register_component<Position>();

  auto entity = reg.spawn_entity();
  auto& comp = reg.emplace_component<Position>(entity, 10.0f, 20.0f);

  REQUIRE(comp.has_value());
  REQUIRE(comp->x == 10.0f);
  REQUIRE(comp->y == 20.0f);
}

TEST_CASE("Registry - removeComponent removes component from entity",
          "[registry]")
{
  Registry reg;
  reg.register_component<Position>();

  auto entity = reg.spawn_entity();
  reg.add_component(entity, Position(10.0f, 20.0f));

  auto& positions = reg.get_components<Position>();
  REQUIRE(positions[entity].has_value());

  reg.remove_component<Position>(entity);

  REQUIRE(!positions[entity].has_value());
}

TEST_CASE("Registry - kill_entity removes all components", "[registry]")
{
  Registry reg;
  reg.register_component<Position>();
  reg.register_component<Velocity>();

  auto entity = reg.spawn_entity();
  reg.add_component(entity, Position(10.0f, 20.0f));
  reg.add_component(entity, Velocity(1.0f, 2.0f));

  auto& positions = reg.get_components<Position>();
  auto& velocities = reg.get_components<Velocity>();

  REQUIRE(positions[entity].has_value());
  REQUIRE(velocities[entity].has_value());

  reg.kill_entity(entity);
  reg.process_entity_deletions();

  REQUIRE(!positions[entity].has_value());
  REQUIRE(!velocities[entity].has_value());
}

TEST_CASE("Registry - kill_entity recycles entity IDs", "[registry]")
{
  Registry reg;

  auto entity1 = reg.spawn_entity();
  auto entity2 = reg.spawn_entity();

  reg.kill_entity(entity1);
  reg.process_entity_deletions();

  auto entity3 = reg.spawn_entity();

  REQUIRE(entity3 == entity1);
}

TEST_CASE("Registry - multiple components per entity", "[registry]")
{
  Registry reg;
  reg.register_component<Position>();
  reg.register_component<Velocity>();
  reg.register_component<Health>();

  auto entity = reg.spawn_entity();
  reg.add_component(entity, Position(10.0f, 20.0f));
  reg.add_component(entity, Velocity(1.0f, 2.0f));
  reg.add_component(entity, Health(100));

  auto& positions = reg.get_components<Position>();
  auto& velocities = reg.get_components<Velocity>();
  auto& healths = reg.get_components<Health>();

  REQUIRE(positions[entity].has_value());
  REQUIRE(velocities[entity].has_value());
  REQUIRE(healths[entity].has_value());
  REQUIRE(healths[entity]->hp == 100);
}

TEST_CASE("Registry - add_system and runSystems", "[registry]")
{
  Registry reg;
  reg.register_component<Position>();

  auto entity = reg.spawn_entity();
  reg.add_component(entity, Position(0.0f, 0.0f));

  int system_runs = 0;

  reg.add_system<Position>(
      [&system_runs](Registry& /*r*/, SparseArray<Position>& /*positions*/)
      { system_runs++; });

  reg.run_systems();

  REQUIRE(system_runs == 1);

  reg.run_systems();

  REQUIRE(system_runs == 2);
}

TEST_CASE("Dummy test", "[dummy]")
{
  REQUIRE(true);
}

struct TestEvent
{
  std::string message;
  int value;
};

struct AnotherEvent
{
  int data;
};

TEST_CASE("Event - on() returns unique handler IDs", "[events]")
{
  Registry reg;

  auto id1 = reg.on<TestEvent>("TestEvent", [](const TestEvent&) {});
  auto id2 = reg.on<TestEvent>("TestEvent", [](const TestEvent&) {});
  auto id3 = reg.on<TestEvent>("TestEvent", [](const TestEvent&) {});

  REQUIRE(id1 != id2);
  REQUIRE(id2 != id3);
  REQUIRE(id1 != id3);
}

TEST_CASE("Event - off() removes specific handler", "[events]")
{
  Registry reg;
  int call_count = 0;

  auto id1 = reg.on<TestEvent>(
      "TestEvent", [&call_count](const TestEvent&) { call_count++; });
  auto id2 = reg.on<TestEvent>(
      "TestEvent", [&call_count](const TestEvent&) { call_count += 10; });

  reg.emit<TestEvent>("test", 1);
  REQUIRE(call_count == 11);

  bool removed = reg.off<TestEvent>(id1);
  REQUIRE(removed);

  call_count = 0;
  reg.emit<TestEvent>("test", 2);
  REQUIRE(call_count == 10);
}

TEST_CASE("Event - off() returns false for non-existent handler", "[events]")
{
  Registry reg;

  bool removed = reg.off<TestEvent>(999);
  REQUIRE_FALSE(removed);
}

TEST_CASE("Event - off() returns false for already removed handler", "[events]")
{
  Registry reg;

  auto id = reg.on<TestEvent>("TestEvent", [](const TestEvent&) {});

  bool removed1 = reg.off<TestEvent>(id);
  REQUIRE(removed1);

  bool removed2 = reg.off<TestEvent>(id);
  REQUIRE_FALSE(removed2);
}

TEST_CASE("Event - off_all() removes all handlers for event type", "[events]")
{
  Registry reg;
  int call_count = 0;

  reg.on<TestEvent>("TestEvent",
                    [&call_count](const TestEvent&) { call_count++; });
  reg.on<TestEvent>("TestEvent",
                    [&call_count](const TestEvent&) { call_count++; });
  reg.on<TestEvent>("TestEvent",
                    [&call_count](const TestEvent&) { call_count++; });

  reg.emit<TestEvent>("test", 1);
  REQUIRE(call_count == 3);

  reg.off_all<TestEvent>();

  call_count = 0;
  reg.emit<TestEvent>("test", 2);
  REQUIRE(call_count == 0);
}

TEST_CASE("Event - off_all() does not affect other event types", "[events]")
{
  Registry reg;
  int test_count = 0;
  int another_count = 0;

  reg.on<TestEvent>("TestEvent",
                    [&test_count](const TestEvent&) { test_count++; });
  reg.on<TestEvent>("TestEvent",
                    [&test_count](const TestEvent&) { test_count++; });
  reg.on<AnotherEvent>([&another_count](const AnotherEvent&)
                       { another_count++; });

  reg.off_all<TestEvent>();

  reg.emit<TestEvent>("test", 1);
  reg.emit<AnotherEvent>(42);

  REQUIRE(test_count == 0);
  REQUIRE(another_count == 1);
}

TEST_CASE("Event - handlers can remove themselves during emission", "[events]")
{
  Registry reg;
  int call_count = 0;
  std::size_t self_handler_id = 0;

  self_handler_id = reg.on<TestEvent>("TestEvent",
                                      [&](const TestEvent&)
                                      {
                                        call_count++;
                                        reg.off<TestEvent>(self_handler_id);
                                      });

  reg.emit<TestEvent>("first", 1);
  REQUIRE(call_count == 1);

  reg.emit<TestEvent>("second", 2);
  REQUIRE(call_count == 1);
}

TEST_CASE("Event - multiple handlers can be removed selectively", "[events]")
{
  Registry reg;
  int count_a = 0;
  int count_b = 0;
  int count_c = 0;
  int count_d = 0;

  auto id1 = reg.on<TestEvent>("TestEvent",
                               [&count_a](const TestEvent&) { count_a++; });
  auto id2 = reg.on<TestEvent>("TestEvent",
                               [&count_b](const TestEvent&) { count_b++; });
  auto id3 = reg.on<TestEvent>("TestEvent",
                               [&count_c](const TestEvent&) { count_c++; });
  auto id4 = reg.on<TestEvent>("TestEvent",
                               [&count_d](const TestEvent&) { count_d++; });

  reg.emit<TestEvent>("test", 1);
  REQUIRE(count_a == 1);
  REQUIRE(count_b == 1);
  REQUIRE(count_c == 1);
  REQUIRE(count_d == 1);

  reg.off<TestEvent>(id2);
  reg.off<TestEvent>(id4);

  reg.emit<TestEvent>("test", 2);
  REQUIRE(count_a == 2);
  REQUIRE(count_b == 1);
  REQUIRE(count_c == 2);
  REQUIRE(count_d == 1);
}

TEST_CASE("Event - handler removal is type-safe", "[events]")
{
  Registry reg;
  int test_count = 0;
  int another_count = 0;

  auto test_id = reg.on<TestEvent>(
      "TestEvent", [&test_count](const TestEvent&) { test_count++; });
  auto another_id = reg.on<AnotherEvent>([&another_count](const AnotherEvent&)
                                         { another_count++; });

  reg.off<AnotherEvent>(test_id);

  reg.emit<TestEvent>("test", 1);
  reg.emit<AnotherEvent>(42);

  REQUIRE(test_count == 1);
  REQUIRE(another_count == 1);
}

TEST_CASE("Event - emit with no handlers does not crash", "[events]")
{
  Registry reg;

  REQUIRE_NOTHROW(reg.emit<TestEvent>("test", 1));

  auto id = reg.on<TestEvent>("TestEvent", [](const TestEvent&) {});
  reg.off<TestEvent>(id);

  REQUIRE_NOTHROW(reg.emit<TestEvent>("test", 2));
}

TEST_CASE("Event - handler IDs are unique", "[events]")
{
  Registry reg;

  auto id1 = reg.on<TestEvent>("TestEvent", [](const TestEvent&) {});
  auto id2 = reg.on<AnotherEvent>([](const AnotherEvent&) {});
  auto id3 = reg.on<TestEvent>("TestEvent", [](const TestEvent&) {});

  REQUIRE(id1 != id2);
  REQUIRE(id2 != id3);
  REQUIRE(id1 != id3);
}

TEST_CASE("Event - can register handler after off_all()", "[events]")
{
  Registry reg;
  int call_count = 0;

  reg.on<TestEvent>("TestEvent",
                    [&call_count](const TestEvent&) { call_count++; });
  reg.off_all<TestEvent>();

  reg.on<TestEvent>("TestEvent",
                    [&call_count](const TestEvent&) { call_count++; });

  reg.emit<TestEvent>("test", 1);
  REQUIRE(call_count == 1);
}

TEST_CASE("Event - all handlers are executed", "[events]")
{
  Registry reg;
  int count1 = 0;
  int count2 = 0;
  int count3 = 0;

  reg.on<TestEvent>("TestEvent", [&count1](const TestEvent&) { count1++; });
  reg.on<TestEvent>("TestEvent", [&count2](const TestEvent&) { count2++; });
  reg.on<TestEvent>("TestEvent", [&count3](const TestEvent&) { count3++; });

  reg.emit<TestEvent>("test", 1);
  REQUIRE(count1 == 1);
  REQUIRE(count2 == 1);
  REQUIRE(count3 == 1);
}

TEST_CASE("Event - removing handler works correctly", "[events]")
{
  Registry reg;
  int count1 = 0;
  int count2 = 0;
  int count3 = 0;

  auto id1 =
      reg.on<TestEvent>("TestEvent", [&count1](const TestEvent&) { count1++; });
  auto id2 =
      reg.on<TestEvent>("TestEvent", [&count2](const TestEvent&) { count2++; });
  auto id3 =
      reg.on<TestEvent>("TestEvent", [&count3](const TestEvent&) { count3++; });

  reg.off<TestEvent>(id2);

  reg.emit<TestEvent>("test", 1);
  REQUIRE(count1 == 1);
  REQUIRE(count2 == 0);
  REQUIRE(count3 == 1);
}

TEST_CASE("Event - handler can access event data", "[events]")
{
  Registry reg;
  std::string received_message;
  int received_value = 0;

  reg.on<TestEvent>("TestEvent",
                    [&](const TestEvent& e)
                    {
                      received_message = e.message;
                      received_value = e.value;
                    });

  reg.emit<TestEvent>("hello", 42);

  REQUIRE(received_message == "hello");
  REQUIRE(received_value == 42);
}

TEST_CASE("Event - multiple emits with handler removal between", "[events]")
{
  Registry reg;
  int call_count = 0;

  auto id = reg.on<TestEvent>(
      "TestEvent", [&call_count](const TestEvent&) { call_count++; });

  reg.emit<TestEvent>("first", 1);
  REQUIRE(call_count == 1);

  reg.emit<TestEvent>("second", 2);
  REQUIRE(call_count == 2);

  reg.off<TestEvent>(id);

  reg.emit<TestEvent>("third", 3);
  REQUIRE(call_count == 2);
}
