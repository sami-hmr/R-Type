#include <any>
#include <optional>
#include <stdexcept>
#include <string>

#include "plugin/Hooks.hpp"

#include <catch2/catch_test_macros.hpp>

#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/HookConcept.hpp"
#include "plugin/HookMacros.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Speed.hpp"
#include "plugin/components/Team.hpp"

// ==================== Test Components ====================

// Simple component with basic hooks
struct SimpleHookable
{
  int value;
  double score;

  HOOKABLE(SimpleHookable, HOOK(value), HOOK(score))
};

// Component with custom hook names
struct CustomHooks
{
  int internal_value;
  std::string data;

  HOOKABLE(CustomHooks,
           HOOK_CUSTOM(val, internal_value),
           HOOK_CUSTOM(info, data))
};

// Component with nested field hooks
struct NestedHooks
{
  Vector2D position;
  int level;

  HOOKABLE(NestedHooks,
           HOOK(position),
           HOOK(position.x),
           HOOK(position.y),
           HOOK(level))
};

// Component with no hooks (empty hook list)
struct EmptyHooks
{
  int private_data;

  HOOKABLE(EmptyHooks, )
};

// Component without HOOKABLE macro (not hookable)
struct NotHookable
{
  int value;
  // No HOOKABLE macro - should not satisfy hookable concept
};

// ==================== Concept Tests ====================

TEST_CASE("Hooks - hookable concept satisfied by HOOKABLE macro",
          "[hooks][concept]")
{
  REQUIRE(hookable<SimpleHookable>);
  REQUIRE(hookable<CustomHooks>);
  REQUIRE(hookable<NestedHooks>);
  REQUIRE(hookable<EmptyHooks>);
  REQUIRE(hookable<Position>);
  REQUIRE(hookable<Speed>);
  REQUIRE(hookable<Team>);
}

TEST_CASE("Hooks - hookable concept not satisfied without HOOKABLE macro",
          "[hooks][concept]")
{
  REQUIRE(!hookable<NotHookable>);
  REQUIRE(!hookable<int>);
  REQUIRE(!hookable<std::string>);
  REQUIRE(!hookable<double>);
}

// ==================== Hook Map Tests ====================

TEST_CASE("Hooks - hook_map returns correct number of hooks",
          "[hooks][hook_map]")
{
  REQUIRE(SimpleHookable::hook_map().size() == 2);
  REQUIRE(CustomHooks::hook_map().size() == 2);
  REQUIRE(NestedHooks::hook_map().size() == 4);
  REQUIRE(EmptyHooks::hook_map().size() == 0);
}

TEST_CASE("Hooks - hook_map contains correct keys", "[hooks][hook_map]")
{
  const auto& simple_hooks = SimpleHookable::hook_map();
  REQUIRE(simple_hooks.find("value") != simple_hooks.end());
  REQUIRE(simple_hooks.find("score") != simple_hooks.end());

  const auto& custom_hooks = CustomHooks::hook_map();
  REQUIRE(custom_hooks.find("val") != custom_hooks.end());
  REQUIRE(custom_hooks.find("info") != custom_hooks.end());
  REQUIRE(custom_hooks.find("internal_value") == custom_hooks.end());
  REQUIRE(custom_hooks.find("data") == custom_hooks.end());

  const auto& nested_hooks = NestedHooks::hook_map();
  REQUIRE(nested_hooks.find("position") != nested_hooks.end());
  REQUIRE(nested_hooks.find("position.x") != nested_hooks.end());
  REQUIRE(nested_hooks.find("position.y") != nested_hooks.end());
  REQUIRE(nested_hooks.find("level") != nested_hooks.end());
}

TEST_CASE("Hooks - hook_map accessor returns correct field",
          "[hooks][hook_map]")
{
  SimpleHookable obj;
  obj.value = 42;
  obj.score = 3.14;

  const auto& hooks = SimpleHookable::hook_map();

  auto value_any = hooks.at("value")(obj);
  auto value_ref = std::any_cast<std::reference_wrapper<int>>(value_any);
  REQUIRE(value_ref.get() == 42);

  auto score_any = hooks.at("score")(obj);
  auto score_ref = std::any_cast<std::reference_wrapper<double>>(score_any);
  REQUIRE(score_ref.get() == 3.14);
}

TEST_CASE("Hooks - hook_map accessor returns reference not copy",
          "[hooks][hook_map]")
{
  SimpleHookable obj;
  obj.value = 100;

  const auto& hooks = SimpleHookable::hook_map();
  auto value_any = hooks.at("value")(obj);
  auto value_ref = std::any_cast<std::reference_wrapper<int>>(value_any);

  // Modify through reference
  value_ref.get() = 200;

  // Original should be modified
  REQUIRE(obj.value == 200);
}

TEST_CASE("Hooks - custom hook names work correctly", "[hooks][hook_map]")
{
  CustomHooks obj;
  obj.internal_value = 99;
  obj.data = "test";

  const auto& hooks = CustomHooks::hook_map();

  auto val_any = hooks.at("val")(obj);
  auto val_ref = std::any_cast<std::reference_wrapper<int>>(val_any);
  REQUIRE(val_ref.get() == 99);

  auto info_any = hooks.at("info")(obj);
  auto info_ref = std::any_cast<std::reference_wrapper<std::string>>(info_any);
  REQUIRE(info_ref.get() == "test");
}

TEST_CASE("Hooks - nested field hooks work correctly", "[hooks][hook_map]")
{
  NestedHooks obj;
  obj.position.x = 10.0;
  obj.position.y = 20.0;
  obj.level = 5;

  const auto& hooks = NestedHooks::hook_map();

  // Test full position hook
  auto pos_any = hooks.at("position")(obj);
  auto pos_ref = std::any_cast<std::reference_wrapper<Vector2D>>(pos_any);
  REQUIRE(pos_ref.get().x == 10.0);
  REQUIRE(pos_ref.get().y == 20.0);

  // Test nested x hook
  auto x_any = hooks.at("position.x")(obj);
  auto x_ref = std::any_cast<std::reference_wrapper<double>>(x_any);
  REQUIRE(x_ref.get() == 10.0);

  // Test nested y hook
  auto y_any = hooks.at("position.y")(obj);
  auto y_ref = std::any_cast<std::reference_wrapper<double>>(y_any);
  REQUIRE(y_ref.get() == 20.0);
}

TEST_CASE("Hooks - empty hook map works correctly", "[hooks][hook_map]")
{
  const auto& hooks = EmptyHooks::hook_map();
  REQUIRE(hooks.empty());
  REQUIRE(hooks.size() == 0);
}

// ==================== Error Cases ====================

TEST_CASE("Hooks - accessing non-existent hook throws", "[hooks][errors]")
{
  SimpleHookable obj;
  const auto& hooks = SimpleHookable::hook_map();

  REQUIRE_THROWS(hooks.at("nonexistent"));
  REQUIRE_THROWS(hooks.at(""));
  REQUIRE_THROWS(hooks.at("invalid_key"));
}

TEST_CASE("Hooks - wrong type cast throws bad_any_cast", "[hooks][errors]")
{
  SimpleHookable obj;
  obj.value = 42;

  const auto& hooks = SimpleHookable::hook_map();
  auto value_any = hooks.at("value")(obj);

  // Try to cast int to wrong type
  REQUIRE_THROWS_AS(std::any_cast<std::reference_wrapper<double>>(value_any),
                    std::bad_any_cast);
  REQUIRE_THROWS_AS(
      std::any_cast<std::reference_wrapper<std::string>>(value_any),
      std::bad_any_cast);
}

TEST_CASE("Hooks - multiple instances have independent hook maps",
          "[hooks][hook_map]")
{
  SimpleHookable obj1;
  SimpleHookable obj2;
  obj1.value = 10;
  obj2.value = 20;

  const auto& hooks = SimpleHookable::hook_map();

  auto val1_any = hooks.at("value")(obj1);
  auto val1_ref = std::any_cast<std::reference_wrapper<int>>(val1_any);

  auto val2_any = hooks.at("value")(obj2);
  auto val2_ref = std::any_cast<std::reference_wrapper<int>>(val2_any);

  REQUIRE(val1_ref.get() == 10);
  REQUIRE(val2_ref.get() == 20);

  // Modify one doesn't affect the other
  val1_ref.get() = 100;
  REQUIRE(obj1.value == 100);
  REQUIRE(obj2.value == 20);
}

// ==================== Integration with Existing Components
// ====================

TEST_CASE("Hooks - Position component hooks work correctly",
          "[hooks][integration]")
{
  Position pos(100.0, 200.0, 5);

  const auto& hooks = Position::hook_map();

  REQUIRE(hooks.size() == 2);
  REQUIRE(hooks.find("pos") != hooks.end());
  REQUIRE(hooks.find("z") != hooks.end());

  auto pos_any = hooks.at("pos")(pos);
  auto pos_ref = std::any_cast<std::reference_wrapper<Vector2D>>(pos_any);
  REQUIRE(pos_ref.get().x == 100.0);
  REQUIRE(pos_ref.get().y == 200.0);

  auto z_any = hooks.at("z")(pos);
  auto z_ref = std::any_cast<std::reference_wrapper<int>>(z_any);
  REQUIRE(z_ref.get() == 5);
}

TEST_CASE("Hooks - Speed component hooks work correctly",
          "[hooks][integration]")
{
  Speed spd(5.0, 10.0);

  const auto& hooks = Speed::hook_map();

  REQUIRE(hooks.size() == 3);  // speed, speed.x, speed.y
  REQUIRE(hooks.find("speed") != hooks.end());
  REQUIRE(hooks.find("speed.x") != hooks.end());
  REQUIRE(hooks.find("speed.y") != hooks.end());

  auto speed_any = hooks.at("speed")(spd);
  auto speed_ref = std::any_cast<std::reference_wrapper<Vector2D>>(speed_any);
  REQUIRE(speed_ref.get().x == 5.0);
  REQUIRE(speed_ref.get().y == 10.0);

  auto x_any = hooks.at("speed.x")(spd);
  auto x_ref = std::any_cast<std::reference_wrapper<double>>(x_any);
  REQUIRE(x_ref.get() == 5.0);

  auto y_any = hooks.at("speed.y")(spd);
  auto y_ref = std::any_cast<std::reference_wrapper<double>>(y_any);
  REQUIRE(y_ref.get() == 10.0);
}

TEST_CASE("Hooks - Team component hooks work correctly", "[hooks][integration]")
{
  Team team("Player");

  const auto& hooks = Team::hook_map();

  REQUIRE(hooks.size() == 1);
  REQUIRE(hooks.find("name") != hooks.end());

  auto name_any = hooks.at("name")(team);
  auto name_ref = std::any_cast<std::reference_wrapper<std::string>>(name_any);
  REQUIRE(name_ref.get() == "Player");
}

// ==================== Edge Cases ====================

TEST_CASE("Hooks - modifying through hook reference updates original",
          "[hooks][edge_cases]")
{
  Position pos(10.0, 20.0);

  const auto& hooks = Position::hook_map();
  auto pos_any = hooks.at("pos")(pos);
  auto pos_ref = std::any_cast<std::reference_wrapper<Vector2D>>(pos_any);

  // Modify through reference
  pos_ref.get().x = 999.0;
  pos_ref.get().y = 888.0;

  // Check original was modified
  REQUIRE(pos.pos.x == 999.0);
  REQUIRE(pos.pos.y == 888.0);
}

TEST_CASE("Hooks - hook map is static and shared", "[hooks][edge_cases]")
{
  const auto& hooks1 = SimpleHookable::hook_map();
  const auto& hooks2 = SimpleHookable::hook_map();

  // Same instance (static)
  REQUIRE(&hooks1 == &hooks2);
}

TEST_CASE("Hooks - hook map is const and cannot be modified",
          "[hooks][edge_cases]")
{
  const auto& hooks = SimpleHookable::hook_map();

  // This should compile because hook_map() returns const reference
  // Attempting to modify would be a compile error
  REQUIRE(hooks.size() == 2);
}

// ==================== Complex Scenarios ====================

TEST_CASE("Hooks - chaining multiple hook accesses", "[hooks][complex]")
{
  NestedHooks obj;
  obj.position.x = 100.0;
  obj.position.y = 200.0;
  obj.level = 10;

  const auto& hooks = NestedHooks::hook_map();

  // Access position
  auto pos_any = hooks.at("position")(obj);
  auto pos_ref = std::any_cast<std::reference_wrapper<Vector2D>>(pos_any);

  // Access x through nested hook
  auto x_any = hooks.at("position.x")(obj);
  auto x_ref = std::any_cast<std::reference_wrapper<double>>(x_any);

  // Both should reference the same value
  REQUIRE(pos_ref.get().x == x_ref.get());

  // Modify through x reference
  x_ref.get() = 500.0;

  // Check both references see the change
  REQUIRE(pos_ref.get().x == 500.0);
  REQUIRE(obj.position.x == 500.0);
}

TEST_CASE("Hooks - using hooks with different component instances",
          "[hooks][complex]")
{
  Position pos1(10.0, 20.0);
  Position pos2(30.0, 40.0);
  Position pos3(50.0, 60.0);

  const auto& hooks = Position::hook_map();

  auto get_x = [&hooks](Position& p) -> double
  {
    auto pos_any = hooks.at("pos")(p);
    auto pos_ref = std::any_cast<std::reference_wrapper<Vector2D>>(pos_any);
    return pos_ref.get().x;
  };

  REQUIRE(get_x(pos1) == 10.0);
  REQUIRE(get_x(pos2) == 30.0);
  REQUIRE(get_x(pos3) == 50.0);
}

TEST_CASE("Hooks - zero values work correctly", "[hooks][edge_cases]")
{
  SimpleHookable obj;
  obj.value = 0;
  obj.score = 0.0;

  const auto& hooks = SimpleHookable::hook_map();

  auto value_any = hooks.at("value")(obj);
  auto value_ref = std::any_cast<std::reference_wrapper<int>>(value_any);
  REQUIRE(value_ref.get() == 0);

  auto score_any = hooks.at("score")(obj);
  auto score_ref = std::any_cast<std::reference_wrapper<double>>(score_any);
  REQUIRE(score_ref.get() == 0.0);
}

TEST_CASE("Hooks - negative values work correctly", "[hooks][edge_cases]")
{
  SimpleHookable obj;
  obj.value = -42;
  obj.score = -3.14;

  const auto& hooks = SimpleHookable::hook_map();

  auto value_any = hooks.at("value")(obj);
  auto value_ref = std::any_cast<std::reference_wrapper<int>>(value_any);
  REQUIRE(value_ref.get() == -42);

  auto score_any = hooks.at("score")(obj);
  auto score_ref = std::any_cast<std::reference_wrapper<double>>(score_any);
  REQUIRE(score_ref.get() == -3.14);
}

TEST_CASE("Hooks - very large values work correctly", "[hooks][edge_cases]")
{
  SimpleHookable obj;
  obj.value = 2147483647;  // INT_MAX
  obj.score = 1e308;  // Large double

  const auto& hooks = SimpleHookable::hook_map();

  auto value_any = hooks.at("value")(obj);
  auto value_ref = std::any_cast<std::reference_wrapper<int>>(value_any);
  REQUIRE(value_ref.get() == 2147483647);

  auto score_any = hooks.at("score")(obj);
  auto score_ref = std::any_cast<std::reference_wrapper<double>>(score_any);
  REQUIRE(score_ref.get() == 1e308);
}
