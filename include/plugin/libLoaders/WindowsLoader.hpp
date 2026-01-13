#pragma once

#ifdef _WIN32

#include <windows.h>
#include <memory>
#include <optional>
#include <string>

#include "ILibLoader.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "plugin/IPlugin.hpp"

template<lib Module>
class WindowsLoader : public LibLoader<Module>
{
public:
  WindowsLoader& operator=(const WindowsLoader&) = delete;
  WindowsLoader& operator=(WindowsLoader&&) = delete;

  explicit WindowsLoader(const std::string& file_name)
      : _lib(LoadLibraryA((file_name + ".dll").c_str()))
  {
    if (this->_lib == nullptr) {
      DWORD err = GetLastError();
      throw NotExistingLib("Error loading library: " + file_name + " (error code: " + std::to_string(err) + ")");
    }
  }

  WindowsLoader(WindowsLoader<Module>&& other) noexcept
      : _lib(other._lib)
  {
    other._lib = nullptr;
  }

  WindowsLoader(WindowsLoader<Module>& other) = delete;

  ~WindowsLoader() override
  {
    if (this->_lib != nullptr) {
      FreeLibrary(this->_lib);
    }
  }

  std::unique_ptr<Module> get_instance(
      const std::string& entry_point,
      Registry& r,
      EventManager& em,
      EntityLoader& e,
      std::optional<JsonObject> const& config) override
  {
    auto* function =
        (IPlugin* (*)(Registry&, EventManager&, EntityLoader&, std::optional<JsonObject> const&))(
            GetProcAddress(this->_lib, entry_point.c_str()));

    if (function == nullptr) {
      throw LoaderException("not a rtype Plugin lib");
    }
    auto* instance = dynamic_cast<Module*>(function(r, em, e, config));
    if (instance == nullptr) {
      throw LoaderException("wrong plugin type");
    }
    std::unique_ptr<Module> tmp(instance);
    return std::move(tmp);
  }

private:
  HINSTANCE _lib = nullptr;
};

#endif // _WIN32
