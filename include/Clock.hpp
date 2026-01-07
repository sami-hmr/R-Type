#pragma once

#include <chrono>

class Clock
{
public:
  using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

  Clock()
      : _now(std::chrono::high_resolution_clock::now())
      , _delta(0)
  {
  }

  void tick()
  {
    TimePoint t = std::chrono::high_resolution_clock::now();
    _delta = t - _now;
    _now = t;
  }

  double delta_seconds() const
  {
    return std::chrono::duration<double>(_delta).count();
  }

  TimePoint now() const { return _now; }

  std::size_t millisecond_now() const
  {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               this->now().time_since_epoch())
        .count();
  }

private:
  TimePoint _now;
  TimePoint::duration _delta;
};
