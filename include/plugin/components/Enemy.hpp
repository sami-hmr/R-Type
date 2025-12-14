#pragma once

#include <string>

struct Enemy
{
  Enemy(std::string enemy_type, bool is_boss)
      : enemy_type(std::move(enemy_type))
      , is_boss(is_boss)
  {
  }
  Enemy(std::string enemy_type)
      : enemy_type(std::move(enemy_type))
      , is_boss(false)
  {
  }

  std::string enemy_type;
  bool is_boss;
};
