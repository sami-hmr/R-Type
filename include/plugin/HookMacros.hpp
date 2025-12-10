#pragma once


#define HOOK_CUSTOM(key, var) \
  {#key, \
   [](Component& self) -> std::any \
   { return std::reference_wrapper(self.var); }}

#define HOOK(var) HOOK_CUSTOM(var, var)

#define HOOKABLE(type, ...) \
  using Component = type; \
  static const auto& hook_map() \
  { \
    static const std::unordered_map<std::string, \
                                    std::function<std::any(type&)>> \
        map {__VA_ARGS__}; \
    return map; \
  }
