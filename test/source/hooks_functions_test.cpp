/**
 * @file hooks_functions_test.cpp
 * @brief Unit tests for hook utility functions (get_ref, get_value_copy,
 * get_value)
 *
 * Tests the Hooks.hpp functions that resolve hook references and extract values
 * from JSON configurations.
 */

#include "plugin/Hooks.hpp"

#include <catch2/catch_test_macros.hpp>

#include "ByteParser/ByteParser.hpp"
#include "Json/JsonParser.hpp"
#include "ParserUtils.hpp"
#include "ecs/Registry.hpp"
#include "libs/Vector2D.hpp"
#include "plugin/Byte.hpp"
#include "plugin/HookMacros.hpp"

// Test components with HOOKABLE macro and serialization
struct TestConfig
{
  TestConfig() = default;

  TestConfig(int h, double s, std::string n)
      : max_health(h)
      , speed(s)
      , name(n)
  {
  }

  int max_health = 100;
  double speed = 5.0;
  std::string name = "default";

  DEFAULT_BYTE_CONSTRUCTOR(TestConfig,
                           ([](int h, double s, std::string n)
                            { return TestConfig {h, s, n}; }),
                           parseByte<int>(),
                           parseByte<double>(),
                           parseByte<std::string>())
  DEFAULT_SERIALIZE(type_to_byte(this->max_health),
                    type_to_byte(this->speed),
                    type_to_byte(this->name))

  HOOKABLE(TestConfig, HOOK(max_health), HOOK(speed), HOOK(name))
};

struct PlayerStats
{
  PlayerStats() = default;

  PlayerStats(int h, int m, Vector2D p = Vector2D {10.0, 20.0})
      : health(h)
      , mana(m)
      , position(p)
  {
  }

  int health = 75;
  int mana = 50;
  Vector2D position {10.0, 20.0};

  DEFAULT_BYTE_CONSTRUCTOR(PlayerStats,
                           ([](int h, int m, Vector2D p)
                            { return PlayerStats {h, m, p}; }),
                           parseByte<int>(),
                           parseByte<int>(),
                           parseByte<Vector2D>())
  DEFAULT_SERIALIZE(type_to_byte(this->health),
                    type_to_byte(this->mana),
                    type_to_byte(this->position))

  HOOKABLE(PlayerStats, HOOK(health), HOOK(mana), HOOK(position))
};

struct WeaponData
{
  WeaponData() = default;

  WeaponData(int d, double fr)
      : damage(d)
      , fire_rate(fr)
  {
  }

  int damage = 25;
  double fire_rate = 2.5;

  DEFAULT_BYTE_CONSTRUCTOR(WeaponData,
                           ([](int d, double fr)
                            { return WeaponData {d, fr}; }),
                           parseByte<int>(),
                           parseByte<double>())
  DEFAULT_SERIALIZE(type_to_byte(this->damage), type_to_byte(this->fire_rate))

  HOOKABLE(WeaponData, HOOK(damage), HOOK(fire_rate))
};

// Component with nested field
struct Transform
{
  Transform() = default;

  Transform(Vector2D p, double r)
      : pos(p)
      , rotation(r)
  {
  }

  Vector2D pos {0.0, 0.0};
  double rotation = 0.0;

  DEFAULT_BYTE_CONSTRUCTOR(Transform,
                           ([](Vector2D p, double r)
                            { return Transform {p, r}; }),
                           parseByte<Vector2D>(),
                           parseByte<double>())
  DEFAULT_SERIALIZE(type_to_byte(this->pos), type_to_byte(this->rotation))

  HOOKABLE(Transform, HOOK(pos), HOOK(rotation))
};

// Component that uses get_value in constructor (for binding tests)
struct Follower
{
  Follower() = default;

  Follower(Registry& r, JsonObject const& obj, Ecs::Entity self)
      : target_pos(get_value<Follower, Vector2D>(r, obj, self, "target_pos")
                       .value_or(Vector2D {0.0, 0.0}))
      , offset_x(get_value_copy<double>(r, obj, "offset_x").value_or(0.0))
  {
  }

  Follower(Vector2D tp, double ox)
      : target_pos(tp)
      , offset_x(ox)
  {
  }

  Vector2D target_pos;
  double offset_x;

  DEFAULT_BYTE_CONSTRUCTOR(Follower,
                           ([](Vector2D tp, double ox)
                            { return Follower {tp, ox}; }),
                           parseByte<Vector2D>(),
                           parseByte<double>())
  DEFAULT_SERIALIZE(type_to_byte(this->target_pos),
                    type_to_byte(this->offset_x))

  HOOKABLE(Follower, HOOK(target_pos), HOOK(offset_x))
};

struct PlayerStats
{
  int health = 75;
  int mana = 50;
  Vector2D position {10.0, 20.0};

  HOOKABLE(PlayerStats, HOOK(health), HOOK(mana), HOOK(position))
};

struct WeaponData
{
  int damage = 25;
  double fire_rate = 2.5;

  HOOKABLE(WeaponData, HOOK(damage), HOOK(fire_rate))
};

// Component with nested field
struct Transform
{
  Vector2D pos {0.0, 0.0};
  double rotation = 0.0;

  HOOKABLE(Transform, HOOK(pos), HOOK(rotation))
};

// Component that uses get_value in constructor (for binding tests)
struct Follower
{
  Vector2D target_pos;
  double offset_x;

  Follower() = default;

  Follower(Registry& r, JsonObject const& obj, Ecs::Entity self)
      : target_pos(get_value<Follower, Vector2D>(r, obj, self, "target_pos")
                       .value_or(Vector2D {0.0, 0.0}))
      , offset_x(get_value_copy<double>(r, obj, "offset_x").value_or(0.0))
  {
  }

  HOOKABLE(Follower, HOOK(target_pos), HOOK(offset_x))
};

// ============================================================================
// get_ref<T>() Tests
// ============================================================================

TEST_CASE("get_ref - Direct JSON value retrieval", "[hooks][get_ref]")
{
  Registry r;
  JsonObject obj;

  SECTION("Get integer value")
  {
    obj["damage"] = 42;
    auto ref = get_ref<int>(r, obj, "damage");

    REQUIRE(ref.has_value());
    REQUIRE(ref->get() == 42);
  }

  SECTION("Get double value")
  {
    obj["speed"] = 3.14;
    auto ref = get_ref<double>(r, obj, "speed");

    REQUIRE(ref.has_value());
    REQUIRE(ref->get() == 3.14);
  }

  SECTION("Get string value")
  {
    obj["name"] = std::string("TestName");
    auto ref = get_ref<std::string>(r, obj, "name");

    REQUIRE(ref.has_value());
    REQUIRE(ref->get() == "TestName");
  }

  SECTION("Get bool value")
  {
    obj["active"] = true;
    auto ref = get_ref<bool>(r, obj, "active");

    REQUIRE(ref.has_value());
    REQUIRE(ref->get() == true);
  }
}

TEST_CASE("get_ref - Missing key returns nullopt", "[hooks][get_ref]")
{
  Registry r;
  JsonObject obj;
  obj["existing"] = 10;

  auto ref = get_ref<int>(r, obj, "missing");
  REQUIRE_FALSE(ref.has_value());
}

TEST_CASE("get_ref - Type mismatch returns nullopt", "[hooks][get_ref]")
{
  Registry r;
  JsonObject obj;
  obj["value"] = 42;

  // Try to get as string instead of int
  auto ref = get_ref<std::string>(r, obj, "value");
  REQUIRE_FALSE(ref.has_value());
}

TEST_CASE("get_ref - Static hook resolution (%)", "[hooks][get_ref]")
{
  Registry r;
  JsonObject obj;

  // Register a TestConfig component with hookable fields
  auto config_entity = r.spawn_entity();
  auto& config = r.add_component<TestConfig>(config_entity, TestConfig {});

  SECTION("Resolve % hook to int field")
  {
    obj["max_hp"] = std::string("%TestConfig:max_health");
    // Note: get_ref only supports # hooks, not % hooks
    // This should return nullopt as % is handled by get_value_copy
    auto ref = get_ref<int>(r, obj, "max_hp");
    REQUIRE_FALSE(ref.has_value());
  }
}

TEST_CASE("get_ref - Dynamic hook resolution (#)", "[hooks][get_ref]")
{
  Registry r;
  JsonObject obj;

  // Register PlayerStats component
  auto player_entity = r.spawn_entity();
  auto& stats =
      r.add_component<PlayerStats>(player_entity, PlayerStats {75, 50});

  SECTION("Resolve # hook to int field")
  {
    obj["current_hp"] = std::string("#PlayerStats:health");
    auto ref = get_ref<int>(r, obj, "current_hp");

    REQUIRE(ref.has_value());
    REQUIRE(ref->get() == 75);
  }

  SECTION("Resolve # hook to Vector2D field")
  {
    obj["pos"] = std::string("#PlayerStats:position");
    auto ref = get_ref<Vector2D>(r, obj, "pos");

    REQUIRE(ref.has_value());
    REQUIRE(ref->get().x == 10.0);
    REQUIRE(ref->get().y == 20.0);
  }

  SECTION("Hook to non-existent component returns nullopt")
  {
    obj["value"] = std::string("#NonExistent:field");
    auto ref = get_ref<int>(r, obj, "value");

    REQUIRE_FALSE(ref.has_value());
  }

  SECTION("Hook to non-existent field returns nullopt")
  {
    obj["value"] = std::string("#PlayerStats:nonexistent");
    auto ref = get_ref<int>(r, obj, "value");

    REQUIRE_FALSE(ref.has_value());
  }
}

TEST_CASE("get_ref - Reference points to actual data", "[hooks][get_ref]")
{
  Registry r;
  JsonObject obj;
  obj["value"] = 100;

  auto ref1 = get_ref<int>(r, obj, "value");
  auto ref2 = get_ref<int>(r, obj, "value");

  REQUIRE(ref1.has_value());
  REQUIRE(ref2.has_value());

  // Both references should point to the same underlying value
  REQUIRE(&(ref1->get()) == &(ref2->get()));
}

// ============================================================================
// get_value_copy<T>() Tests
// ============================================================================

TEST_CASE("get_value_copy - Direct JSON value retrieval",
          "[hooks][get_value_copy]")
{
  Registry r;
  JsonObject obj;

  SECTION("Copy integer value")
  {
    obj["damage"] = 42;
    auto value = get_value_copy<int>(r, obj, "damage");

    REQUIRE(value.has_value());
    REQUIRE(value.value() == 42);
  }

  SECTION("Copy double value")
  {
    obj["speed"] = 3.14;
    auto value = get_value_copy<double>(r, obj, "speed");

    REQUIRE(value.has_value());
    REQUIRE(value.value() == 3.14);
  }

  SECTION("Copy string value")
  {
    obj["name"] = std::string("TestName");
    auto value = get_value_copy<std::string>(r, obj, "name");

    REQUIRE(value.has_value());
    REQUIRE(value.value() == "TestName");
  }

  SECTION("Copy bool value")
  {
    obj["active"] = true;
    auto value = get_value_copy<bool>(r, obj, "active");

    REQUIRE(value.has_value());
    REQUIRE(value.value() == true);
  }
}

TEST_CASE("get_value_copy - Static hook resolution (%)",
          "[hooks][get_value_copy]")
{
  Registry r;
  JsonObject obj;

  // Register TestConfig component
  auto config_entity = r.spawn_entity();
  r.add_component<TestConfig>(config_entity, TestConfig {100, 5.0, "default"});

  SECTION("Resolve % hook to int field")
  {
    obj["max_hp"] = std::string("%TestConfig:max_health");
    auto value = get_value_copy<int>(r, obj, "max_hp");

    // Note: % hooks are resolved by get_value, not get_value_copy
    // get_value_copy will try get_ref first which returns nullopt for %
    REQUIRE_FALSE(value.has_value());
  }
}

TEST_CASE("get_value_copy - Dynamic hook resolution (#)",
          "[hooks][get_value_copy]")
{
  Registry r;
  JsonObject obj;

  // Register PlayerStats component
  auto player_entity = r.spawn_entity();
  r.add_component<PlayerStats>(player_entity,
                               PlayerStats {75, 50, Vector2D {10.0, 20.0}});

  SECTION("Resolve # hook to int field")
  {
    obj["hp"] = std::string("#PlayerStats:health");
    auto value = get_value_copy<int>(r, obj, "hp");

    REQUIRE(value.has_value());
    REQUIRE(value.value() == 75);
  }

  SECTION("Resolve # hook to Vector2D field")
  {
    obj["pos"] = std::string("#PlayerStats:position");
    auto value = get_value_copy<Vector2D>(r, obj, "pos");

    REQUIRE(value.has_value());
    REQUIRE(value->x == 10.0);
    REQUIRE(value->y == 20.0);
  }
}

TEST_CASE("get_value_copy - Returns independent copy",
          "[hooks][get_value_copy]")
{
  Registry r;
  JsonObject obj;

  auto player_entity = r.spawn_entity();
  auto& stats =
      r.add_component<PlayerStats>(player_entity, PlayerStats {75, 50});

  obj["hp"] = std::string("#PlayerStats:health");

  auto value1 = get_value_copy<int>(r, obj, "hp");
  auto value2 = get_value_copy<int>(r, obj, "hp");

  REQUIRE(value1.has_value());
  REQUIRE(value2.has_value());
  REQUIRE(value1.value() == value2.value());

  // Modify the original component
  stats.health = 100;

  // Copies should still have old value
  REQUIRE(value1.value() == 75);
  REQUIRE(value2.value() == 75);
}

TEST_CASE("get_value_copy - Missing key returns nullopt",
          "[hooks][get_value_copy]")
{
  Registry r;
  JsonObject obj;
  obj["existing"] = 10;

  auto value = get_value_copy<int>(r, obj, "missing");
  REQUIRE_FALSE(value.has_value());
}

TEST_CASE("get_value_copy - value_or provides defaults",
          "[hooks][get_value_copy]")
{
  Registry r;
  JsonObject obj;

  SECTION("Existing key returns actual value")
  {
    obj["speed"] = 5.0;
    double speed = get_value_copy<double>(r, obj, "speed").value_or(10.0);
    REQUIRE(speed == 5.0);
  }

  SECTION("Missing key returns default")
  {
    double speed = get_value_copy<double>(r, obj, "missing").value_or(10.0);
    REQUIRE(speed == 10.0);
  }
}

TEST_CASE("get_value_copy - JSON object construction",
          "[hooks][get_value_copy]")
{
  Registry r;
  JsonObject obj;

  SECTION("Construct Vector2D from JSON")
  {
    JsonObject vec_obj;
    vec_obj["x"] = 3.0;
    vec_obj["y"] = 4.0;
    obj["position"] = vec_obj;

    auto pos = get_value_copy<Vector2D>(r, obj, "position");

    REQUIRE(pos.has_value());
    REQUIRE(pos->x == 3.0);
    REQUIRE(pos->y == 4.0);
  }
}

// ============================================================================
// get_value<ComponentType, T>() Tests
// ============================================================================

TEST_CASE("get_value - Direct JSON value retrieval", "[hooks][get_value]")
{
  Registry r;
  JsonObject obj;
  auto entity = r.spawn_entity();

  SECTION("Get integer value")
  {
    obj["damage"] = 42;
    auto value = get_value<WeaponData, int>(r, obj, entity, "damage");

    REQUIRE(value.has_value());
    REQUIRE(value.value() == 42);
  }

  SECTION("Get double value")
  {
    obj["speed"] = 3.14;
    auto value = get_value<WeaponData, double>(r, obj, entity, "speed");

    REQUIRE(value.has_value());
    REQUIRE(value.value() == 3.14);
  }

  SECTION("Get string value")
  {
    obj["name"] = std::string("Sword");
    auto value = get_value<WeaponData, std::string>(r, obj, entity, "name");

    REQUIRE(value.has_value());
    REQUIRE(value.value() == "Sword");
  }
}

TEST_CASE("get_value - Static hook resolution (%)", "[hooks][get_value]")
{
  Registry r;
  JsonObject obj;

  // Register TestConfig component
  auto config_entity = r.spawn_entity();
  r.add_component<TestConfig>(config_entity, TestConfig {100, 5.0, "default"});

  auto entity = r.spawn_entity();

  SECTION("Resolve % hook to int field")
  {
    obj["max_hp"] = std::string("%TestConfig:max_health");
    auto value = get_value<PlayerStats, int>(r, obj, entity, "max_hp");

    REQUIRE(value.has_value());
    REQUIRE(value.value() == 100);
  }

  SECTION("Resolve % hook to double field")
  {
    obj["speed"] = std::string("%TestConfig:speed");
    auto value = get_value<PlayerStats, double>(r, obj, entity, "speed");

    REQUIRE(value.has_value());
    REQUIRE(value.value() == 5.0);
  }

  SECTION("Resolve % hook to string field")
  {
    obj["config_name"] = std::string("%TestConfig:name");
    auto value =
        get_value<PlayerStats, std::string>(r, obj, entity, "config_name");

    REQUIRE(value.has_value());
    REQUIRE(value.value() == "default");
  }
}

TEST_CASE("get_value - Dynamic hook resolution (#)", "[hooks][get_value]")
{
  Registry r;
  JsonObject obj;

  // Register PlayerStats component
  auto player_entity = r.spawn_entity();
  r.add_component<PlayerStats>(player_entity,
                               PlayerStats {75, 50, Vector2D {10.0, 20.0}});

  auto follower_entity = r.spawn_entity();

  SECTION("Resolve # hook to int field")
  {
    obj["target_hp"] = std::string("#PlayerStats:health");
    auto value = get_value<Follower, int>(r, obj, follower_entity, "target_hp");

    REQUIRE(value.has_value());
    REQUIRE(value.value() == 75);
    // Note: Binding should be registered but we don't verify it here
  }

  SECTION("Resolve # hook to Vector2D field")
  {
    obj["target_pos"] = std::string("#PlayerStats:position");
    auto value =
        get_value<Follower, Vector2D>(r, obj, follower_entity, "target_pos");

    REQUIRE(value.has_value());
    REQUIRE(value->x == 10.0);
    REQUIRE(value->y == 20.0);
  }
}

TEST_CASE("get_value - Dynamic binding registration", "[hooks][get_value]")
{
  Registry r;
  JsonObject obj;

  // Register source component
  auto source_entity = r.spawn_entity();
  auto& source_stats =
      r.add_component<PlayerStats>(source_entity, PlayerStats {100, 50});

  // Create follower with dynamic hook
  auto follower_entity = r.spawn_entity();
  obj["target_pos"] = std::string("#PlayerStats:position");

  // This should register a binding
  auto value =
      get_value<Follower, Vector2D>(r, obj, follower_entity, "target_pos");

  REQUIRE(value.has_value());
  REQUIRE(value->x == 10.0);
  REQUIRE(value->y == 20.0);

  // Modify source position
  source_stats.position.x = 100.0;
  source_stats.position.y = 200.0;

  // The value we got is a copy, so it won't change
  // But the binding should be registered internally
  // (We can't easily test binding without accessing Registry internals)
}

TEST_CASE("get_value - Missing key returns nullopt", "[hooks][get_value]")
{
  Registry r;
  JsonObject obj;
  auto entity = r.spawn_entity();

  obj["existing"] = 10;
  auto value = get_value<WeaponData, int>(r, obj, entity, "missing");

  REQUIRE_FALSE(value.has_value());
}

TEST_CASE("get_value - Hook to non-existent component", "[hooks][get_value]")
{
  Registry r;
  JsonObject obj;
  auto entity = r.spawn_entity();

  SECTION("# hook to non-existent component returns empty T{}")
  {
    obj["value"] = std::string("#NonExistent:field");
    auto value = get_value<WeaponData, int>(r, obj, entity, "value");

    // According to implementation, returns T{} (empty value, not nullopt)
    REQUIRE(value.has_value());
    REQUIRE(value.value() == 0);  // Default int{}
  }

  SECTION("% hook to non-existent component returns nullopt")
  {
    obj["value"] = std::string("%NonExistent:field");
    auto value = get_value<WeaponData, int>(r, obj, entity, "value");

    REQUIRE_FALSE(value.has_value());
  }
}

TEST_CASE("get_value - value_or provides defaults", "[hooks][get_value]")
{
  Registry r;
  JsonObject obj;
  auto entity = r.spawn_entity();

  SECTION("Existing key returns actual value")
  {
    obj["damage"] = 50;
    int damage =
        get_value<WeaponData, int>(r, obj, entity, "damage").value_or(10);
    REQUIRE(damage == 50);
  }

  SECTION("Missing key returns default")
  {
    int damage =
        get_value<WeaponData, int>(r, obj, entity, "missing").value_or(10);
    REQUIRE(damage == 10);
  }
}

// ============================================================================
// is_hook() Tests
// ============================================================================

TEST_CASE("is_hook - Detect dynamic hooks", "[hooks][is_hook]")
{
  JsonObject obj;

  SECTION("# prefix is a dynamic hook")
  {
    obj["target"] = std::string("#Player:pos");
    REQUIRE(is_hook(obj, "target") == true);
  }

  SECTION("% prefix is NOT detected as hook")
  {
    obj["config"] = std::string("%Config:value");
    REQUIRE(is_hook(obj, "config") == false);
  }

  SECTION("Direct value is not a hook")
  {
    obj["damage"] = 42;
    REQUIRE(is_hook(obj, "damage") == false);
  }

  SECTION("String without prefix is not a hook")
  {
    obj["name"] = std::string("test");
    REQUIRE(is_hook(obj, "name") == false);
  }

  SECTION("Missing key is not a hook")
  {
    REQUIRE(is_hook(obj, "missing") == false);
  }
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST_CASE("Hooks - Full component construction with get_value",
          "[hooks][integration]")
{
  Registry r;

  // Create source component
  auto player_entity = r.spawn_entity();
  r.add_component<Transform>(player_entity,
                             Transform {Vector2D {50.0, 100.0}, 45.0});

  // Create follower JSON config
  JsonObject follower_config;
  follower_config["target_pos"] = std::string("#Transform:pos");
  follower_config["offset_x"] = 10.0;

  // Spawn follower using config
  auto follower_entity = r.spawn_entity();
  auto& follower = r.add_component<Follower>(
      follower_entity, Follower(r, follower_config, follower_entity));

  // Check that values were correctly initialized
  REQUIRE(follower.target_pos.x == 50.0);
  REQUIRE(follower.target_pos.y == 100.0);
  REQUIRE(follower.offset_x == 10.0);
}

TEST_CASE("Hooks - Multiple components with different hook types",
          "[hooks][integration]")
{
  Registry r;

  // Create config component
  auto config_entity = r.spawn_entity();
  r.add_component<TestConfig>(config_entity,
                              TestConfig {200, 7.5, "PlayerConfig"});

  // Create player component
  auto player_entity = r.spawn_entity();
  r.add_component<PlayerStats>(player_entity,
                               PlayerStats {200, 100, Vector2D {0.0, 0.0}});

  // Create weapon with mixed hooks
  JsonObject weapon_config;
  weapon_config["damage"] = std::string("%TestConfig:max_health");  // Static
  weapon_config["fire_rate"] = 1.5;  // Direct

  auto weapon_entity = r.spawn_entity();

  auto damage =
      get_value<WeaponData, int>(r, weapon_config, weapon_entity, "damage");
  auto fire_rate = get_value<WeaponData, double>(
      r, weapon_config, weapon_entity, "fire_rate");

  REQUIRE(damage.has_value());
  REQUIRE(damage.value() == 200);  // From TestConfig::max_health
  REQUIRE(fire_rate.has_value());
  REQUIRE(fire_rate.value() == 1.5);  // Direct value
}

TEST_CASE("Hooks - Chaining hook references", "[hooks][integration]")
{
  Registry r;

  // Create chain: Config -> PlayerStats -> Follower
  auto config_entity = r.spawn_entity();
  r.add_component<TestConfig>(config_entity, TestConfig {150, 5.0, "config"});

  auto player_entity = r.spawn_entity();
  r.add_component<PlayerStats>(player_entity, PlayerStats {150, 75});

  JsonObject follower_config;
  follower_config["target_pos"] = std::string("#PlayerStats:position");
  follower_config["offset_x"] = 5.0;

  auto follower_entity = r.spawn_entity();
  auto& follower = r.add_component<Follower>(
      follower_entity, Follower(r, follower_config, follower_entity));

  // Follower should have PlayerStats position
  REQUIRE(follower.target_pos.x == 10.0);  // Default PlayerStats position
  REQUIRE(follower.target_pos.y == 20.0);
}

TEST_CASE("Hooks - Error handling with value_or", "[hooks][error_handling]")
{
  Registry r;
  JsonObject obj;
  auto entity = r.spawn_entity();

  SECTION("Missing keys with defaults")
  {
    int health =
        get_value<PlayerStats, int>(r, obj, entity, "health").value_or(100);
    double speed =
        get_value<PlayerStats, double>(r, obj, entity, "speed").value_or(5.0);
    std::string name =
        get_value<PlayerStats, std::string>(r, obj, entity, "name")
            .value_or("unknown");

    REQUIRE(health == 100);
    REQUIRE(speed == 5.0);
    REQUIRE(name == "unknown");
  }

  SECTION("Invalid hooks with defaults")
  {
    obj["bad_hook"] = std::string("%NonExistent:field");
    int value =
        get_value<PlayerStats, int>(r, obj, entity, "bad_hook").value_or(999);

    REQUIRE(value == 999);
  }
}
