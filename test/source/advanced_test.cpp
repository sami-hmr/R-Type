#include <chrono>
#include <thread>

#include <catch2/catch_test_macros.hpp>

#include "Clock.hpp"
#include "TwoWayMap.hpp"

TEST_CASE("TwoWayMap - bidirectional lookup", "[twowaymap]")
{
  TwoWayMap<int, std::string> map;

  map.insert(1, "one");
  map.insert(2, "two");
  map.insert(3, "three");

  REQUIRE(map.at_first(1) == "one");
  REQUIRE(map.at_first(2) == "two");
  REQUIRE(map.at_first(3) == "three");

  REQUIRE(map.at_second("one") == 1);
  REQUIRE(map.at_second("two") == 2);
  REQUIRE(map.at_second("three") == 3);
}

TEST_CASE("TwoWayMap - initializer list constructor", "[twowaymap]")
{
  TwoWayMap<int, std::string> map {{1, "one"}, {2, "two"}, {3, "three"}};

  REQUIRE(map.at(1) == "one");
  REQUIRE(map.at("two") == 2);
  REQUIRE(map.at(3) == "three");
}

TEST_CASE("TwoWayMap - updates existing values", "[twowaymap]")
{
  TwoWayMap<int, std::string> map;

  map.insert(1, "one");
  map.insert(1, "uno");

  REQUIRE(map.at_first(1) == "uno");
  REQUIRE(map.at_second("uno") == 1);
}

TEST_CASE("TwoWayMap - throws on missing key", "[twowaymap]")
{
  TwoWayMap<int, std::string> map;
  map.insert(1, "one");

  REQUIRE_THROWS(map.at_first(999));
  REQUIRE_THROWS(map.at_second("nonexistent"));
}

TEST_CASE("TwoWayMap - overwrites update forward mapping", "[twowaymap]")
{
  TwoWayMap<int, std::string> map;

  map.insert(1, "one");
  map.insert(2, "two");

  map.insert(1, "uno");

  REQUIRE(map.at_first(1) == "uno");
  REQUIRE(map.at_second("uno") == 1);
  // Note: TwoWayMap doesn't clean up old reverse mappings on overwrite
  // This is a known limitation of the current implementation
}

TEST_CASE("Clock - delta_seconds increases over time", "[clock]")
{
  Clock clock;

  clock.tick();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  clock.tick();

  double delta = clock.delta_seconds();
  REQUIRE(delta > 0.0);
  REQUIRE(delta < 1.0);
}

TEST_CASE("Clock - multiple ticks", "[clock]")
{
  Clock clock;

  for (int i = 0; i < 5; i++) {
    clock.tick();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  double delta = clock.delta_seconds();
  REQUIRE(delta > 0.0);
}

TEST_CASE("Clock - now returns current time", "[clock]")
{
  Clock clock;
  auto t1 = clock.now();

  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  auto t2 = std::chrono::high_resolution_clock::now();

  REQUIRE(t2 > t1);
}
