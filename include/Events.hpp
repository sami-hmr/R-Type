#pragma once

#include <string>

struct ShutdownEvent
{
  std::string reason;
  int exit_code = 0;
};

struct CleanupEvent
{
  std::string trigger;
};
