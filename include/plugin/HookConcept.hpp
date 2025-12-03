#pragma once

#include <any>
#include <concepts>
#include <functional>
#include <string>
#include <unordered_map>

template<typename T>
concept hookable = requires(T& t) {
  {
    t.hook_map
  } -> std::same_as<std::unordered_map<std::string, std::function<std::any ()>>&>;
};
