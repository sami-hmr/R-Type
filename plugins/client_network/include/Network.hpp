#pragma once

#include <vector>

#include "ecs/Registery.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"

class NetworkClient : public APlugin
{
public:
  NetworkClient(Registery& r, EntityLoader& l)
      : APlugin(r, l, {}, {})
  {
  }

private:
};
