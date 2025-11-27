#pragma once

#include <string>

/**
 * @brief Event triggered when the application should shut down.
 */
struct ShutdownEvent
{
  std::string reason;
  int exit_code = 0;
};

/**
 * @brief Event triggered during cleanup operations.
 */
struct CleanupEvent
{
  std::string trigger;
};
