#pragma once

#include <any>
#include <concepts>
#include <functional>
#include <string>
#include <unordered_map>
template<typename T>
concept hookable = requires() {
  {
    T::hook_map()
  } -> std::same_as<const std ::unordered_map<std ::string, std ::function<std ::any(T&)>>&>;
};
