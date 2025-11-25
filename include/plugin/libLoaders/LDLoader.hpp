#pragma once

#include <memory>
#include <string>

#include <dlfcn.h>

#include "ILibLoader.hpp"
#include "ecs/Registery.hpp"
#include "plugin/IPlugin.hpp"
#include "plugin/libLoaders/ILibLoader.hpp"

template<lib Module>
class DlLoader : public LibLoader<Module>
{
public:
  DlLoader& operator=(const DlLoader&) = delete;
  DlLoader& operator=(DlLoader&&) = delete;

  explicit DlLoader(const std::string& file_name)
      : _lib(dlopen((file_name + ".so").c_str(), RTLD_LAZY))
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
    if (this->_lib != nullptr) {
      (void)dlclose(this->_lib);
    }
  }

  std::unique_ptr<Module> get_instance(const std::string &entry_point,
                                      Registery& r,
                                      EntityLoader& e) override
  {
    auto *function = (IPlugin* (*)(Registery&, EntityLoader&))
        (dlsym(this->_lib, entry_point.c_str()));

    if (function == nullptr) {
      throw LoaderException("not a rtype Plugin lib");
    }
    auto *instance = dynamic_cast<Module*>(function(r, e));
    if (instance == nullptr) {
      throw LoaderException("wrong plugin type");
    }
    std::unique_ptr<Module> tmp(instance);
    return std::move(tmp);
  }

private:
  void* _lib = nullptr;
};
