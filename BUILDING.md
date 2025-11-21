# Building with CMake

## Quick Start

The fastest way to build this project is using CMake presets:

```sh
cmake --preset=dev          # Configure
cmake --build --preset=dev  # Build
./build/dev/r-type          # Run
```

## Project Structure

This project builds two main components:
- **r-type_lib** - An OBJECT library containing the core game engine code
- **r-type_exe** - An executable that links against and uses the library

You can build them separately:
```sh
# Build only the library
cmake --build --preset=dev --target r-type_lib

# Build only the executable
cmake --build --preset=dev --target r-type_exe

# Build everything
cmake --build --preset=dev
```

## Dependencies

For a list of runtime dependencies, please refer to [vcpkg.json](vcpkg.json).

### Required Tools

- CMake 3.14 or later
- C++17 compatible compiler (GCC or Clang)
- make (or another build system supported by CMake)
- vcpkg (for dependency management in manifest mode)

### Runtime Dependencies

These are automatically managed by vcpkg and listed in `vcpkg.json`:
- **fmt** (>= 11.0.2) - Fast and safe C++ formatting library

### Optional Development Dependencies

These tools are not required to build the project but provide additional
functionality for code quality, testing, and documentation:

#### Code Quality Tools

- **cppcheck** - C/C++ static code analyzer
  - Detects bugs, undefined behavior, and dangerous coding constructs
  - Usage: `cppcheck --enable=all --suppress=missingIncludeSystem source/`
  - Install: `sudo apt install cppcheck` (Debian/Ubuntu) or `brew install cppcheck` (macOS)

- **clang-format** - Code formatting tool
  - Automatically formats code according to style rules (see `.clang-format`)
  - Usage: `clang-format -i source/*.cpp source/*.hpp`
  - Install: `sudo apt install clang-format` (Debian/Ubuntu) or `brew install clang-format` (macOS)

- **clang-tidy** - Clang-based C++ linter
  - Provides additional static analysis and modernization checks
  - Integrated into CMake preset when available
  - Install: `sudo apt install clang-tidy` (Debian/Ubuntu) or `brew install llvm` (macOS)

#### Spell Checking

- **codespell** - Fixes common misspellings in text files
  - Checks source code, comments, and documentation
  - Configuration in `.codespellrc`
  - Usage: `codespell source/`
  - Install: `pip install codespell` or `sudo apt install codespell`

#### Documentation

- **doxygen** - Documentation generator
  - Generates API documentation from annotated source code
  - Configuration in `docs/Doxyfile.in`
  - Usage: Enable `BUILD_MCSS_DOCS` in CMake and build `docs` target
  - Install: `sudo apt install doxygen` (Debian/Ubuntu) or `brew install doxygen` (macOS)

- **graphviz** - Graph visualization software (required by Doxygen)
  - Used to generate class diagrams and dependency graphs
  - Install: `sudo apt install graphviz` (Debian/Ubuntu) or `brew install graphviz` (macOS)

- **m.css** - Modern CSS framework for documentation
  - Provides modern styling for Doxygen output
  - Automatically used when building documentation
  - Requires Python 3 and Jinja2

#### Testing

- **Catch2** (>= 3.7.1) - C++ testing framework
  - Required only when `r-type_DEVELOPER_MODE` is ON
  - Automatically installed by vcpkg when enabled
  - Listed in `vcpkg.json` under the `test` feature

#### Debugging and Profiling

- **gdb** - GNU Debugger
  - Interactive debugger for C/C++ programs
  - Usage: `gdb ./build/dev/r-type`
  - Install: `sudo apt install gdb` (Debian/Ubuntu) or `brew install gdb` (macOS)

- **valgrind** - Memory debugging and profiling tool
  - Detects memory leaks and memory management bugs
  - Usage: `valgrind --leak-check=full ./build/dev/r-type`
  - Install: `sudo apt install valgrind` (Debian/Ubuntu) or `brew install valgrind` (macOS)

### Installing All Development Tools

**Debian/Ubuntu:**
```sh
sudo apt update
sudo apt install cmake g++ make cppcheck clang-format clang-tidy \
                 codespell doxygen graphviz gdb valgrind python3-pip
pip3 install jinja2 Pygments
```

**macOS:**
```sh
brew install cmake llvm cppcheck clang-format doxygen graphviz gdb
pip3 install codespell jinja2 Pygments
```

**Arch Linux:**
```sh
sudo pacman -S cmake gcc make cppcheck clang doxygen graphviz gdb valgrind codespell python-jinja python-pygments
```

## Manual Build

### Development Build

Here are the steps for building in debug mode with presets:

```sh
cmake --preset=dev
cmake --build --preset=dev
```

### Release Build

Here are the steps for building in release mode with a single-configuration
generator, like the Unix Makefiles one:

```sh
cmake -S . -B build -D CMAKE_BUILD_TYPE=Release
cmake --build build
```

Here are the steps for building in release mode with a multi-configuration
generator, like the Visual Studio ones:

```sh
cmake -S . -B build
cmake --build build --config Release
```

### Building Specific Targets

Build only the library:
```sh
cmake --build build --target r-type_lib
```

Build only the executable:
```sh
cmake --build build --target r-type_exe
```

### Building with MSVC

Note that MSVC by default is not standards compliant and you need to pass some
flags to make it behave properly. See the `flags-msvc` preset in the
[CMakePresets.json](CMakePresets.json) file for the flags and with what
variable to provide them to CMake during configuration.

### Building on Apple Silicon

CMake supports building on Apple Silicon properly since 3.20.1. Make sure you
have the [latest version][1] installed.

## Install

This project doesn't require any special command-line flags to install to keep
things simple. As a prerequisite, the project has to be built with the above
commands already.

The below commands require at least CMake 3.15 to run, because that is the
version in which [Install a Project][2] was added.

Here is the command for installing the release mode artifacts with a
single-configuration generator, like the Unix Makefiles one:

```sh
cmake --install build
```

Here is the command for installing the release mode artifacts with a
multi-configuration generator, like the Visual Studio ones:

```sh
cmake --install build --config Release
```

## Troubleshooting

### Clang linking errors

If you encounter undefined reference errors with Clang, the CMakeLists.txt has been
configured to automatically link against libstdc++. If issues persist, ensure you have
the C++ standard library development packages installed.

### Missing VCPKG_ROOT

The project uses vcpkg manifest mode. Dependencies are managed via vcpkg.json and
installed automatically during CMake configuration.

[1]: https://cmake.org/download/
[2]: https://cmake.org/cmake/help/latest/manual/cmake.1.html#install-a-project
