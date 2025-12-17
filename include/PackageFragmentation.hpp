#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

struct FragmentedPackage
{
  class Hash
  {
  public:
    std::size_t operator()(FragmentedPackage const& d) const
    {
      std::size_t r = d.sequence;
      std::size_t tmp = d.id;
      r += tmp << (sizeof(d.sequence) * 8);
      std::hash<std::size_t> hash;
      return hash(r);
    }
  };

  bool operator==(FragmentedPackage const& o) const
  {
    return (o.id == this->id) && (o.sequence == this->sequence);
  }

  std::uint32_t sequence;
  std::uint8_t id;
};
