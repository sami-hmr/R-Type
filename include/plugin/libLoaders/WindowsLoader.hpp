// /*
// ** EPITECH PROJECT, 2025
// ** raytraycer
// ** File description:
// ** lib loaders
// */

// #ifndef WINDOWS_LOADER_HPP
// #define WINDOWS_LOADER_HPP

// #include <unistd.h>
// #include <windows.h>

// #include <libconfig.h++>
// #include <memory>
// #include <string>

// #include "libLoaders/ILibLoader.hpp"
// #include "plugins/IPlugin.hpp"

// template <lib Module>
// class WindowsLoader : public LibLoader<Module> {
//    public:
//     WindowsLoader(std::string fileName) : lib_(LoadLibrary(fileName)) {
//         if (!this->lib_) {
//             throw NotExistingLib("error loading library" + fileName);
//         }
//     }

//     WindowsLoader(WindowsLoader<Module> &&other) : lib_(other.lib_) {
//         other.lib_ = nullptr;
//     }

//     WindowsLoader(WindowsLoader<Module> &other) = delete;
//     ~WindowsLoader() override {
//     }

//     std::unique_ptr<Module> get_instance(
//         const std::string entryPoint,
//         const libconfig::Setting &settings) override {
//         IPlugin *(*function)(const libconfig::Setting &) =
//             (IPlugin * (*)(const libconfig::Setting &))(
//                 GetProcAddress(this->lib_, entryPoint));

//         if (!function) {
//             throw LoaderException("not a raytracerPlugin lib");
//         }
//         Module *instance = dynamic_cast<Module *>(function(settings));
//         if (!instance) {
//             throw LoaderException("wrong plugin type");
//         }
//         std::unique_ptr<Module> tmp(instance);
//         return std::move(tmp);
//     }

//    private:
//     HINSTANCE lib_ = nullptr;
// };

// #endif
