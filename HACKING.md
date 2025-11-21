# Hacking

Here is some wisdom to help you build and test this project as a developer and
potential contributor.

If you plan to contribute, please read the [CONTRIBUTING](CONTRIBUTING.md)
guide.

## Quick Start for Developers

To get started with development:

```sh
cmake --preset=dev          # Configure
cmake --build --preset=dev  # Build
./build/dev/r-type          # Run
```

### Common Development Tasks

```sh
# Make changes to library code
vim source/lib.cpp

# Rebuild just the library (faster)
cmake --build --preset=dev --target r-type_lib

# Rebuild everything
cmake --build --preset=dev

# Run the executable
./build/dev/r-type

# Check code quality
cppcheck --enable=all --suppress=missingIncludeSystem source/

# Debug
gdb ./build/dev/r-type
valgrind --leak-check=full ./build/dev/r-type
```

## Project Architecture

This project uses a library + executable pattern:
- **r-type_lib** (OBJECT library) - Contains all core game engine logic
- **r-type_exe** (executable) - Thin wrapper that uses the library

This separation allows you to:
- Build and test the library independently
- Link the library into multiple executables (client, server, etc.)
- Keep the main logic separate from entry point code

Add your code to the library in `source/lib.cpp` and `source/lib.hpp`.

## Developer mode

Build system targets that are only useful for developers of this project are
hidden if the `r-type_DEVELOPER_MODE` option is disabled. Enabling this
option makes tests and other developer targets and options available. Not
enabling this option means that you are a consumer of this project and thus you
have no need for these targets and options.

Developer mode is always set to on in CI workflows.

**Note:** The current `CMakeUserPresets.json` has developer mode disabled to avoid
requiring test dependencies (Catch2). To enable full developer features including
tests, documentation, and additional targets, modify `CMakeUserPresets.json` to
set `r-type_DEVELOPER_MODE` to `ON`.

## Development Environment Setup

### Presets

This project makes use of [presets][1] to simplify the process of configuring
the project. As a developer, you are recommended to always have the [latest
CMake version][2] installed to make use of the latest Quality-of-Life
additions.

A `CMakeUserPresets.json` file is already included at the root of
the project. You can modify it to customize your development workflow.

The preset inherits from the base presets defined in `CMakePresets.json`.

`CMakeUserPresets.json` is also the perfect place in which you can put all
sorts of things that you would otherwise want to pass to the configure command
in the terminal.

### Dependency manager

This project uses [vcpkg][vcpkg] in manifest mode. Dependencies are declared in
`vcpkg.json` and automatically installed during CMake configuration.

For a detailed list of all dependencies including optional development tools,
see the Dependencies section in [BUILDING.md](BUILDING.md).

[vcpkg]: https://github.com/microsoft/vcpkg

### Development Tools Setup

To get the full development experience with all code quality tools, install the
optional development dependencies. These tools are not required to build the
project but are highly recommended for contributors.

**Quick install (Debian/Ubuntu):**
```sh
sudo apt install cppcheck clang-format clang-tidy codespell doxygen graphviz gdb valgrind
```

See [BUILDING.md](BUILDING.md#optional-development-dependencies) for a complete
list of tools with descriptions and installation instructions for all platforms.

### Configure, build and test

You can configure, build and test the project respectively with the following 
commands from the project root on any operating system with any build system:

```sh
cmake --preset=dev
cmake --build --preset=dev
ctest --preset=dev          # Requires developer mode ON
```

If you are using a compatible editor (e.g. VSCode) or IDE (e.g. CLion, VS), you
will also be able to select the above created user presets for automatic
integration.

Please note that both the build and test commands accept a `-j` flag to specify
the number of jobs to use, which should ideally be specified to the number of
threads your CPU has. You may also want to add that to your preset using the
`jobs` property, see the [presets documentation][1] for more details.

### Developer mode targets

These are targets you may invoke using the build command from above, with an
additional `-t <target>` flag:

#### `r-type_lib`

Builds only the core library. Useful for faster iteration when working on library code:
```sh
cmake --build --preset=dev -t r-type_lib
```

#### `r-type_exe`

Builds only the executable (and its dependencies):
```sh
cmake --build --preset=dev -t r-type_exe
```

#### `coverage`

Available if `ENABLE_COVERAGE` is enabled and developer mode is ON. This target processes the output of
the previously run tests when built with coverage configuration. The commands
this target runs can be found in the `COVERAGE_TRACE_COMMAND` and
`COVERAGE_HTML_COMMAND` cache variables. The trace command produces an info
file by default, which can be submitted to services with CI integration. The
HTML command uses the trace command's output to generate an HTML document to
`<binary-dir>/coverage_html` by default.

#### `docs`

Available if `BUILD_MCSS_DOCS` is enabled and developer mode is ON. Builds documentation using
Doxygen and m.css. The output will go to `<binary-dir>/docs` by default
(customizable using `DOXYGEN_OUTPUT_DIRECTORY`).

#### `format-check` and `format-fix`

These targets run the clang-format tool on the codebase to check errors and to
fix them respectively. Customization available using the `FORMAT_PATTERNS` and
`FORMAT_COMMAND` cache variables.

#### `run-exe`

Runs the executable target `r-type_exe`. Available when developer mode is ON.

#### `spell-check` and `spell-fix`

These targets run the codespell tool on the codebase to check errors and to fix
them respectively. Customization available using the `SPELL_COMMAND` cache
variable.

## Code Quality Tools

The project supports several code quality tools:

### Static Analysis
```sh
cppcheck --enable=all --suppress=missingIncludeSystem source/
```

### Formatting
```sh
clang-format -i source/*.cpp source/*.hpp
```

### Spell Checking
```sh
codespell source/
```

[1]: https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html
[2]: https://cmake.org/download/
