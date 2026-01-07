#pragma once

#include <memory>
#include <optional>
#include <string>

#include <dlfcn.h>

#include "ILibLoader.hpp"
#include "Json/JsonParser.hpp"
#include "ecs/EventManager.hpp"
#include "ecs/Registry.hpp"
#include "plugin/IPlugin.hpp"
#include "plugin/libLoaders/ILibLoader.hpp"

template<lib Module>
class DlLoader : public LibLoader<Module>
{
public:
  DlLoader& operator=(const DlLoader&) = delete;
  DlLoader& operator=(DlLoader&&) = delete;

  explicit DlLoader(const std::string& file_name)
      : hey(file_name)
      , _lib(dlopen((file_name + ".so").c_str(), RTLD_LAZY))
  {
    if (this->_lib == nullptr) {
      throw NotExistingLib(dlerror());
    }
  }

  DlLoader(DlLoader<Module>&& other) noexcept
      : _lib(other._lib)
  {
    other._lib = nullptr;
  }

  DlLoader(DlLoader<Module>& other) = delete;

  ~DlLoader() override
  {
    // NOTE: We intentionally do NOT call dlclose() here.
    // Reason: Plugins may have registered static/global destructors, TLS
    // destructors, or atexit handlers that contain code from the .so file. If
    // we dlclose() here, those destructors will crash when they try to execute
    // code from unmapped memory. The OS will clean up the loaded libraries when
    // the process exits.
    //
    // This is a known limitation of dynamic library unloading in C++.
    // See: https://stackoverflow.com/questions/7977869/segfault-on-dlclose

    // if (this->_lib != nullptr) {
    //   dlclose(this->_lib);
    // }
    _lib = nullptr;  // Mark as cleaned up
  }

  std::unique_ptr<Module> get_instance(
      const std::string& entry_point,
      Registry& r,
      EventManager& em,
      EntityLoader& e,
      std::optional<JsonObject> const& config) override
  {
    auto* function = (IPlugin
                      * (*)(Registry&,
                            EventManager&,
                            EntityLoader&,
                            std::optional<JsonObject> const&))(
        dlsym(this->_lib, entry_point.c_str()));

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
  std::string hey;
  void* _lib = nullptr;
};
