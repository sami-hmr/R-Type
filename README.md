# r-type

This is the r-type project - an ECS (Entity Component System) game engine.

## Project Structure

This project uses a library + executable architecture:
- **r-type_lib** - Main library containing the game engine code
- **r-type_exe** - Executable wrapper that uses the library

## Quick Start

### Building the Project

```bash
cmake --preset=dev          # Configure with dev preset
cmake --build --preset=dev  # Build everything
./build/dev/r-type          # Run the executable
```

### Building Specific Targets

```bash
# Build only the library
cmake --build --preset=dev --target r-type_lib

# Build only the executable
cmake --build --preset=dev --target r-type_exe
```

### Release Build

```bash
cmake -S . -B build -D CMAKE_BUILD_TYPE=Release
cmake --build build --config Release
cmake --install build --config Release
```

# Building and installing

See the [BUILDING](BUILDING.md) document.

# Contributing

See the [CONTRIBUTING](CONTRIBUTING.md) document.

# Licensing

[MIT License](LICENSE)

