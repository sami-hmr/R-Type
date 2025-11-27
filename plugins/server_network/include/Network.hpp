#pragma once

#include <vector>

#include "ecs/Registery.hpp"
#include "plugin/APlugin.hpp"
#include "plugin/EntityLoader.hpp"

class NetworkServer : public APlugin
{
public:
  NetworkServer(Registery& r, EntityLoader& l)
      : APlugin(r, l, {}, {})
  {
  }

private:
};
