#pragma once

template<typename T>
concept hookable = requires(T& t) {
  {
    t.hook_map
  };
};
