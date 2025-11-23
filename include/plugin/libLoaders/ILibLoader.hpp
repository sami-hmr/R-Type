/*
** EPITECH PROJECT, 2025
** raytracer
** File description:
** interface for plugin loader
*/

#ifndef LIB_LOADER_HPP
#define LIB_LOADER_HPP

#include <memory>

#include <libconfig.h++>

#include "CustomException.hpp"
#include "ecs/Registery.hpp"
#include "plugin/IPlugin.hpp"

class EntityLoader;

class NotExistingLib : public CustomException
{
public:
  NotExistingLib(std::string message)
      : CustomException(message)
  {
  }
};

class LoaderNotExistingFunction : public CustomException
{
public:
  LoaderNotExistingFunction(std::string message)
      : CustomException(message)
  {
  }
};

class LoaderException : public CustomException
{
public:
  LoaderException(std::string message)
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
  virtual std::unique_ptr<Module> getInstance(const std::string entryPoint,
                                              Registery& r,
                                              EntityLoader& e) = 0;
};

#endif
