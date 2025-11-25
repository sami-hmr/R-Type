#pragma once
#include <string>

#include "CustomException.hpp"
#include "ecs/Registery.hpp"
#include "plugin/IPlugin.hpp"

class EntityLoader;

class NotExistingLib : public CustomException
{
public:
  explicit NotExistingLib(const std::string& message)
      : CustomException(message)
  {
  }
};

class LoaderNotExistingFunction : public CustomException
{
public:
  explicit LoaderNotExistingFunction(const std::string& message)
      : CustomException(message)
  {
  }
};

class LoaderException : public CustomException
{
public:
  explicit LoaderException(const std::string& message)
      : CustomException(message)
  {
  }
};

template<typename Module>
concept lib = std::is_base_of<IPlugin, Module>::value;

template<lib Module>
class LibLoader
{
public:
  virtual ~LibLoader() = default;
  virtual std::unique_ptr<Module> get_instance(const std::string& entry_point,
                                               Registery& r,
                                               EntityLoader& e) = 0;
};
