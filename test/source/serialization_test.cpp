#include <limits>

#include <catch2/catch_test_macros.hpp>

#include "plugin/Byte.hpp"
#include "plugin/components/Position.hpp"
#include "plugin/components/Sprite.hpp"
#include "plugin/components/Team.hpp"
#include "plugin/components/Text.hpp"
#include "plugin/components/Velocity.hpp"
#include "plugin/events/Events.hpp"

// ==================== Serialization Tests ====================

TEST_CASE("Serialization - type_to_byte with int32_t", "[serialization]")
{
  std::int32_t value = 42;
  ByteArray bytes = type_to_byte(value);

  REQUIRE(bytes.size() == sizeof(std::int32_t));
}

TEST_CASE("Serialization - type_to_byte with negative int32_t",
          "[serialization]")
{
  std::int32_t value = -12345;
  ByteArray bytes = type_to_byte(value);

  REQUIRE(bytes.size() == sizeof(std::int32_t));
}

TEST_CASE("Serialization - type_to_byte with double", "[serialization]")
{
  double value = 3.14159;
  ByteArray bytes = type_to_byte(value);

  REQUIRE(bytes.size() == sizeof(double));
}

TEST_CASE("Serialization - type_to_byte with float", "[serialization]")
{
  float value = 2.71828f;
  ByteArray bytes = type_to_byte(value);

  REQUIRE(bytes.size() == sizeof(float));
}

TEST_CASE("Serialization - type_to_byte with zero", "[serialization]")
{
  std::int32_t value = 0;
  ByteArray bytes = type_to_byte(value);

  REQUIRE(bytes.size() == sizeof(std::int32_t));
}

TEST_CASE("Serialization - type_to_byte with max int32_t", "[serialization]")
{
  std::int32_t value = std::numeric_limits<std::int32_t>::max();
  ByteArray bytes = type_to_byte(value);

  REQUIRE(bytes.size() == sizeof(std::int32_t));
}

TEST_CASE("Serialization - type_to_byte with min int32_t", "[serialization]")
{
  std::int32_t value = std::numeric_limits<std::int32_t>::min();
  ByteArray bytes = type_to_byte(value);

  REQUIRE(bytes.size() == sizeof(std::int32_t));
}

TEST_CASE("Serialization - type_to_byte with uint8_t", "[serialization]")
{
  std::uint8_t value = 255;
  ByteArray bytes = type_to_byte(value);

  REQUIRE(bytes.size() == sizeof(std::uint8_t));
  REQUIRE(bytes[0] == 255);
}

TEST_CASE("Serialization - type_to_byte with uint16_t", "[serialization]")
{
  std::uint16_t value = 65535;
  ByteArray bytes = type_to_byte(value);

  REQUIRE(bytes.size() == sizeof(std::uint16_t));
}

TEST_CASE("Serialization - type_to_byte with uint64_t", "[serialization]")
{
  std::uint64_t value = 0xFFFFFFFFFFFFFFFF;
  ByteArray bytes = type_to_byte(value);

  REQUIRE(bytes.size() == sizeof(std::uint64_t));
}

TEST_CASE("Serialization - string_to_byte with regular string",
          "[serialization]")
{
  std::string str = "Hello, World!";
  ByteArray bytes = string_to_byte(str);

  // Should contain size (4 bytes) + string content
  REQUIRE(bytes.size() == sizeof(std::uint32_t) + str.size());
}

TEST_CASE("Serialization - string_to_byte with empty string", "[serialization]")
{
  std::string str = "";
  ByteArray bytes = string_to_byte(str);

  // Should only contain size (4 bytes for 0)
  REQUIRE(bytes.size() == sizeof(std::uint32_t));
}

TEST_CASE("Serialization - string_to_byte with long string", "[serialization]")
{
  std::string str(1000, 'x');
  ByteArray bytes = string_to_byte(str);

  REQUIRE(bytes.size() == sizeof(std::uint32_t) + 1000);
}

TEST_CASE("Serialization - string_to_byte with special characters",
          "[serialization]")
{
  std::string str = "Test\n\t\r!@#$%^&*()";
  ByteArray bytes = string_to_byte(str);

  REQUIRE(bytes.size() == sizeof(std::uint32_t) + str.size());
}

TEST_CASE("Serialization - ByteArray concatenation operator", "[serialization]")
{
  ByteArray first = {1, 2, 3};
  ByteArray second = {4, 5, 6};

  ByteArray result = first + second;

  REQUIRE(result.size() == 6);
  REQUIRE(result[0] == 1);
  REQUIRE(result[3] == 4);
  REQUIRE(result[5] == 6);
}

TEST_CASE("Serialization - ByteArray concatenation with empty arrays",
          "[serialization]")
{
  ByteArray first = {1, 2, 3};
  ByteArray empty;

  ByteArray result1 = first + empty;
  ByteArray result2 = empty + first;

  REQUIRE(result1.size() == 3);
  REQUIRE(result2.size() == 3);
}

TEST_CASE("Serialization - ByteArray concatenation chaining", "[serialization]")
{
  ByteArray a = {1};
  ByteArray b = {2};
  ByteArray c = {3};
  ByteArray d = {4};

  ByteArray result = a + b + c + d;

  REQUIRE(result.size() == 4);
  REQUIRE(result[0] == 1);
  REQUIRE(result[1] == 2);
  REQUIRE(result[2] == 3);
  REQUIRE(result[3] == 4);
}

TEST_CASE("Serialization - Position to_bytes", "[serialization]")
{
  Position pos(10.5, 20.5);
  ByteArray bytes = pos.to_bytes();

  // Should contain 2 doubles
  REQUIRE(bytes.size() == 2 * sizeof(double));
}

TEST_CASE("Serialization - Position round-trip", "[serialization]")
{
  Position original(123.456, 789.012);
  ByteArray bytes = original.to_bytes();
  Position deserialized(bytes);

  REQUIRE(deserialized.pos.x == original.pos.x);
  REQUIRE(deserialized.pos.y == original.pos.y);
}

TEST_CASE("Serialization - Position with zero values", "[serialization]")
{
  Position original(0.0, 0.0);
  ByteArray bytes = original.to_bytes();
  Position deserialized(bytes);

  REQUIRE(deserialized.pos.x == 0.0);
  REQUIRE(deserialized.pos.y == 0.0);
}

TEST_CASE("Serialization - Position with negative values", "[serialization]")
{
  Position original(-50.5, -100.75);
  ByteArray bytes = original.to_bytes();
  Position deserialized(bytes);

  REQUIRE(deserialized.pos.x == original.pos.x);
  REQUIRE(deserialized.pos.y == original.pos.y);
}

TEST_CASE("Serialization - Position with large values", "[serialization]")
{
  Position original(999999.999, -999999.999);
  ByteArray bytes = original.to_bytes();
  Position deserialized(bytes);

  REQUIRE(deserialized.pos.x == original.pos.x);
  REQUIRE(deserialized.pos.y == original.pos.y);
}

TEST_CASE("Serialization - Position with very small values", "[serialization]")
{
  Position original(0.0000001, -0.0000001);
  ByteArray bytes = original.to_bytes();
  Position deserialized(bytes);

  REQUIRE(deserialized.pos.x == original.pos.x);
  REQUIRE(deserialized.pos.y == original.pos.y);
}

TEST_CASE("Serialization - Position with extreme double values",
          "[serialization]")
{
  Position original(std::numeric_limits<double>::max() / 2.0,
                    std::numeric_limits<double>::min() * 2.0);
  ByteArray bytes = original.to_bytes();
  Position deserialized(bytes);

  REQUIRE(deserialized.pos.x == original.pos.x);
  REQUIRE(deserialized.pos.y == original.pos.y);
}

TEST_CASE("Serialization - Velocity to_bytes", "[serialization]")
{
  Velocity vel(1.0, 2.0, 3.0, 4.0);
  ByteArray bytes = vel.to_bytes();

  // Should contain 4 doubles
  REQUIRE(bytes.size() == 4 * sizeof(double));
}

TEST_CASE("Serialization - Velocity round-trip", "[serialization]")
{
  Velocity original(10.5, 20.5, 0.707, 0.707);
  ByteArray bytes = original.to_bytes();
  Velocity deserialized(bytes);

  REQUIRE(deserialized.speed.x == original.speed.x);
  REQUIRE(deserialized.speed.y == original.speed.y);
  REQUIRE(deserialized.direction.x == original.direction.x);
  REQUIRE(deserialized.direction.y == original.direction.y);
}

TEST_CASE("Serialization - Velocity with zero speed", "[serialization]")
{
  Velocity original(0.0, 0.0, 1.0, 0.0);
  ByteArray bytes = original.to_bytes();
  Velocity deserialized(bytes);

  REQUIRE(deserialized.speed.x == 0.0);
  REQUIRE(deserialized.speed.y == 0.0);
  REQUIRE(deserialized.direction.x == 1.0);
  REQUIRE(deserialized.direction.y == 0.0);
}

TEST_CASE("Serialization - Velocity with negative values", "[serialization]")
{
  Velocity original(-5.5, -10.5, -0.6, -0.8);
  ByteArray bytes = original.to_bytes();
  Velocity deserialized(bytes);

  REQUIRE(deserialized.speed.x == original.speed.x);
  REQUIRE(deserialized.speed.y == original.speed.y);
  REQUIRE(deserialized.direction.x == original.direction.x);
  REQUIRE(deserialized.direction.y == original.direction.y);
}

TEST_CASE("Serialization - Velocity all zeros", "[serialization]")
{
  Velocity original(0.0, 0.0, 0.0, 0.0);
  ByteArray bytes = original.to_bytes();
  Velocity deserialized(bytes);

  REQUIRE(deserialized.speed.x == 0.0);
  REQUIRE(deserialized.speed.y == 0.0);
  REQUIRE(deserialized.direction.x == 0.0);
  REQUIRE(deserialized.direction.y == 0.0);
}

TEST_CASE("Serialization - Velocity with fractional values", "[serialization]")
{
  Velocity original(0.123456789, 9.876543210, 0.707106781, -0.707106781);
  ByteArray bytes = original.to_bytes();
  Velocity deserialized(bytes);

  REQUIRE(deserialized.speed.x == original.speed.x);
  REQUIRE(deserialized.speed.y == original.speed.y);
  REQUIRE(deserialized.direction.x == original.direction.x);
  REQUIRE(deserialized.direction.y == original.direction.y);
}

TEST_CASE("Serialization - Team to_bytes", "[serialization]")
{
  Team team("Player");
  ByteArray bytes = team.to_bytes();

  // Should contain size (4 bytes) + string content
  REQUIRE(bytes.size() == sizeof(std::uint32_t) + 6);
}

TEST_CASE("Serialization - Team round-trip", "[serialization]")
{
  Team original("Allies");
  ByteArray bytes = original.to_bytes();
  Team deserialized(bytes);

  REQUIRE(deserialized.name == original.name);
}

TEST_CASE("Serialization - Team with empty name", "[serialization]")
{
  Team original("");
  ByteArray bytes = original.to_bytes();
  Team deserialized(bytes);

  REQUIRE(deserialized.name == "");
}

TEST_CASE("Serialization - Team with single character", "[serialization]")
{
  Team original("A");
  ByteArray bytes = original.to_bytes();
  Team deserialized(bytes);

  REQUIRE(deserialized.name == "A");
  REQUIRE(deserialized.name.size() == 1);
}

TEST_CASE("Serialization - Team with long name", "[serialization]")
{
  std::string long_name(500, 'x');
  Team original(long_name);
  ByteArray bytes = original.to_bytes();
  Team deserialized(bytes);

  REQUIRE(deserialized.name == long_name);
  REQUIRE(deserialized.name.size() == 500);
}

TEST_CASE("Serialization - Team with special characters", "[serialization]")
{
  Team original("Team-Alpha_123!");
  ByteArray bytes = original.to_bytes();
  Team deserialized(bytes);

  REQUIRE(deserialized.name == "Team-Alpha_123!");
}

TEST_CASE("Serialization - Team with numbers in name", "[serialization]")
{
  Team original("Team123");
  ByteArray bytes = original.to_bytes();
  Team deserialized(bytes);

  REQUIRE(deserialized.name == "Team123");
}

TEST_CASE("Serialization - Sprite to_bytes", "[serialization]")
{
  Sprite sprite("assets/test.png", Vector2D {2.0, 2.0});
  ByteArray bytes = sprite.to_bytes();

  // Should contain: string size + string content + 2 doubles
  REQUIRE(bytes.size() > sizeof(std::uint32_t) + 2 * sizeof(double));
}

TEST_CASE("Serialization - Sprite round-trip", "[serialization]")
{
  Sprite original("assets/player.png", Vector2D {1.5, 1.5});
  ByteArray bytes = original.to_bytes();
  Sprite deserialized(bytes);

  REQUIRE(deserialized.texture_path == original.texture_path);
  REQUIRE(deserialized.scale.x == original.scale.x);
  REQUIRE(deserialized.scale.y == original.scale.y);
}

TEST_CASE("Serialization - Sprite with empty texture path", "[serialization]")
{
  Sprite original("", Vector2D {1.0, 1.0});
  ByteArray bytes = original.to_bytes();
  Sprite deserialized(bytes);

  REQUIRE(deserialized.texture_path == "");
  REQUIRE(deserialized.scale.x == 1.0);
  REQUIRE(deserialized.scale.y == 1.0);
}

TEST_CASE("Serialization - Sprite with zero scale", "[serialization]")
{
  Sprite original("assets/tiny.png", Vector2D {0.0, 0.0});
  ByteArray bytes = original.to_bytes();
  Sprite deserialized(bytes);

  REQUIRE(deserialized.texture_path == "assets/tiny.png");
  REQUIRE(deserialized.scale.x == 0.0);
  REQUIRE(deserialized.scale.y == 0.0);
}

TEST_CASE("Serialization - Sprite with negative scale", "[serialization]")
{
  Sprite original("assets/flipped.png", Vector2D {-1.0, 2.0});
  ByteArray bytes = original.to_bytes();
  Sprite deserialized(bytes);

  REQUIRE(deserialized.texture_path == "assets/flipped.png");
  REQUIRE(deserialized.scale.x == -1.0);
  REQUIRE(deserialized.scale.y == 2.0);
}

TEST_CASE("Serialization - Sprite with long path", "[serialization]")
{
  std::
      string
          long_path =
              "assets/very/long/path/to/some/texture/file/that/is/deeply/"
              "nested/" "sprite.png";
  Sprite original(long_path, Vector2D {1.0, 1.0});
  ByteArray bytes = original.to_bytes();
  Sprite deserialized(bytes);

  REQUIRE(deserialized.texture_path == long_path);
}

TEST_CASE("Serialization - Sprite with path containing spaces",
          "[serialization]")
{
  Sprite original("assets/my texture file.png", Vector2D {1.0, 2.0});
  ByteArray bytes = original.to_bytes();
  Sprite deserialized(bytes);

  REQUIRE(deserialized.texture_path == "assets/my texture file.png");
}

TEST_CASE("Serialization - Sprite with large scale values", "[serialization]")
{
  Sprite original("sprite.png", Vector2D {1000.0, 2000.0});
  ByteArray bytes = original.to_bytes();
  Sprite deserialized(bytes);

  REQUIRE(deserialized.scale.x == 1000.0);
  REQUIRE(deserialized.scale.y == 2000.0);
}

TEST_CASE("Serialization - Text to_bytes", "[serialization]")
{
  Text text("assets/font.ttf", Vector2D {1.0, 1.0}, "Hello");
  ByteArray bytes = text.to_bytes();

  // Should contain: 2 string sizes + 2 string contents + 2 doubles
  REQUIRE(bytes.size() > 2 * sizeof(std::uint32_t) + 2 * sizeof(double));
}

TEST_CASE("Serialization - Text round-trip", "[serialization]")
{
  Text original("assets/roboto.ttf", Vector2D {2.0, 2.5}, "Test Message");
  ByteArray bytes = original.to_bytes();
  Text deserialized(bytes);

  REQUIRE(deserialized.font_path == original.font_path);
  REQUIRE(deserialized.scale.x == original.scale.x);
  REQUIRE(deserialized.scale.y == original.scale.y);
  REQUIRE(deserialized.text == original.text);
}

TEST_CASE("Serialization - Text with empty strings", "[serialization]")
{
  Text original("", Vector2D {1.0, 1.0}, "");
  ByteArray bytes = original.to_bytes();
  Text deserialized(bytes);

  REQUIRE(deserialized.font_path == "");
  REQUIRE(deserialized.text == "");
}

TEST_CASE("Serialization - Text with long content", "[serialization]")
{
  std::string long_text(1000, 'A');
  Text original("font.ttf", Vector2D {1.0, 1.0}, long_text);
  ByteArray bytes = original.to_bytes();
  Text deserialized(bytes);

  REQUIRE(deserialized.text == long_text);
  REQUIRE(deserialized.text.size() == 1000);
}

TEST_CASE("Serialization - Text with special characters", "[serialization]")
{
  Text original("font.ttf", Vector2D {1.0, 1.0}, "Hello\nWorld\t!");
  ByteArray bytes = original.to_bytes();
  Text deserialized(bytes);

  REQUIRE(deserialized.text == "Hello\nWorld\t!");
}

TEST_CASE("Serialization - Text with unicode-like characters",
          "[serialization]")
{
  Text original("font.ttf", Vector2D {1.5, 1.5}, "Test: !@#$%^&*()");
  ByteArray bytes = original.to_bytes();
  Text deserialized(bytes);

  REQUIRE(deserialized.text == "Test: !@#$%^&*()");
}

TEST_CASE("Serialization - Text with only whitespace", "[serialization]")
{
  Text original("font.ttf", Vector2D {1.0, 1.0}, "   \t\n   ");
  ByteArray bytes = original.to_bytes();
  Text deserialized(bytes);

  REQUIRE(deserialized.text == "   \t\n   ");
}

TEST_CASE("Serialization - CliComp to_bytes", "[serialization]")
{
  CliComp cli;
  ByteArray bytes = cli.to_bytes();

  // CliComp serializes to empty byte array
  REQUIRE(bytes.empty());
}

TEST_CASE("Serialization - CliComp round-trip", "[serialization]")
{
  CliComp original;
  ByteArray bytes = original.to_bytes();
  CliComp deserialized(bytes);

  // No specific fields to check, just verify it doesn't throw
  REQUIRE(bytes.empty());
}

TEST_CASE("Serialization - multiple Position serialization", "[serialization]")
{
  Position pos1(10.0, 20.0);
  Position pos2(30.0, 40.0);
  Position pos3(50.0, 60.0);

  ByteArray bytes1 = pos1.to_bytes();
  ByteArray bytes2 = pos2.to_bytes();
  ByteArray bytes3 = pos3.to_bytes();

  Position deser1(bytes1);
  Position deser2(bytes2);
  Position deser3(bytes3);

  REQUIRE(deser1.pos.x == 10.0);
  REQUIRE(deser1.pos.y == 20.0);
  REQUIRE(deser2.pos.x == 30.0);
  REQUIRE(deser2.pos.y == 40.0);
  REQUIRE(deser3.pos.x == 50.0);
  REQUIRE(deser3.pos.y == 60.0);
}

TEST_CASE("Serialization - byte_array_join with multiple arrays",
          "[serialization]")
{
  ByteArray arr1 = {1, 2};
  ByteArray arr2 = {3, 4};
  ByteArray arr3 = {5, 6};

  ByteArray result = byte_array_join(arr1, arr2, arr3);

  REQUIRE(result.size() == 6);
  REQUIRE(result[0] == 1);
  REQUIRE(result[2] == 3);
  REQUIRE(result[4] == 5);
}

TEST_CASE("Serialization - invalid ByteArray for Position throws",
          "[serialization]")
{
  ByteArray invalid_bytes = {1, 2, 3};  // Too short for Position

  REQUIRE_THROWS_AS(Position(invalid_bytes), InvalidPackage);
}

TEST_CASE("Serialization - empty ByteArray for Position throws",
          "[serialization]")
{
  ByteArray empty_bytes;

  REQUIRE_THROWS_AS(Position(empty_bytes), InvalidPackage);
}

TEST_CASE("Serialization - invalid ByteArray for Velocity throws",
          "[serialization]")
{
  ByteArray invalid_bytes = {1, 2, 3, 4};  // Too short for Velocity

  REQUIRE_THROWS_AS(Velocity(invalid_bytes), InvalidPackage);
}

TEST_CASE("Serialization - empty ByteArray for Velocity throws",
          "[serialization]")
{
  ByteArray empty_bytes;

  REQUIRE_THROWS_AS(Velocity(empty_bytes), InvalidPackage);
}

TEST_CASE("Serialization - invalid ByteArray for Team throws",
          "[serialization]")
{
  ByteArray invalid_bytes = {255, 255, 255, 255};  // Invalid size

  REQUIRE_THROWS_AS(Team(invalid_bytes), InvalidPackage);
}

TEST_CASE("Serialization - empty ByteArray for Team throws", "[serialization]")
{
  ByteArray empty_bytes;

  REQUIRE_THROWS_AS(Team(empty_bytes), InvalidPackage);
}

TEST_CASE("Serialization - invalid ByteArray for Sprite throws",
          "[serialization]")
{
  ByteArray invalid_bytes = {1, 2};  // Too short

  REQUIRE_THROWS_AS(Sprite(invalid_bytes), InvalidPackage);
}

TEST_CASE("Serialization - empty ByteArray for Sprite throws",
          "[serialization]")
{
  ByteArray empty_bytes;

  REQUIRE_THROWS_AS(Sprite(empty_bytes), InvalidPackage);
}

TEST_CASE("Serialization - invalid ByteArray for Text throws",
          "[serialization]")
{
  ByteArray invalid_bytes = {1, 2, 3};  // Too short

  REQUIRE_THROWS_AS(Text(invalid_bytes), InvalidPackage);
}

TEST_CASE("Serialization - empty ByteArray for Text throws", "[serialization]")
{
  ByteArray empty_bytes;

  REQUIRE_THROWS_AS(Text(empty_bytes), InvalidPackage);
}

TEST_CASE("Serialization - multiple consecutive serializations",
          "[serialization]")
{
  for (int i = 0; i < 10; i++) {
    Position pos(static_cast<double>(i), static_cast<double>(i * 2));
    ByteArray bytes = pos.to_bytes();
    Position deser(bytes);

    REQUIRE(deser.pos.x == static_cast<double>(i));
    REQUIRE(deser.pos.y == static_cast<double>(i * 2));
  }
}

TEST_CASE("Serialization - mixed component serialization", "[serialization]")
{
  Position pos(100.0, 200.0);
  Velocity vel(1.0, 2.0, 0.707, 0.707);
  Team team("TestTeam");

  ByteArray pos_bytes = pos.to_bytes();
  ByteArray vel_bytes = vel.to_bytes();
  ByteArray team_bytes = team.to_bytes();

  Position pos_deser(pos_bytes);
  Velocity vel_deser(vel_bytes);
  Team team_deser(team_bytes);

  REQUIRE(pos_deser.pos.x == 100.0);
  REQUIRE(vel_deser.speed.x == 1.0);
  REQUIRE(team_deser.name == "TestTeam");
}
