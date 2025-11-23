# Building with CMake

## Quick Start

```sh
cmake --preset=dev
cmake --build build
./r-type
```

## Build Requirements

- CMake 3.14 or later
- C++23 compatible compiler (GCC 12+, Clang 15+)
- Git (for fetching dependencies)

### Optional: Coverage Reports

For generating HTML coverage reports:
- lcov
- perl-gd (Perl GD module)

### Optional: Documentation Coverage

For generating documentation coverage reports:
- Python 3
- coverxygen (`pip3 install coverxygen`)
- lcov (for HTML reports)

### Installing Dependencies

**Debian/Ubuntu:**
```sh
sudo apt update
sudo apt install cmake g++ git

# For coverage reports:
sudo apt install lcov libgd-perl

# For documentation coverage:
pip3 install coverxygen
```

**Arch Linux:**
```sh
sudo pacman -S cmake gcc git

# For coverage reports:
sudo pacman -S lcov perl-gd

# For documentation coverage:
pip3 install coverxygen
```

**macOS:**
```sh
brew install cmake git

# For coverage reports:
brew install lcov
cpan install GD

# For documentation coverage:
pip3 install coverxygen
```

## Build Presets

This project uses CMake presets for different build configurations:

### Development Build

Development build includes test coverage instrumentation:

```sh
cmake --preset=dev
cmake --build build
```

### Release Build

```sh
cmake --preset=release
cmake --build build/release
```

### Coverage Report

The dev build includes coverage by default:

```sh
cmake --preset=dev
cmake --build build
ctest --test-dir build
cmake --build build --target coverage
```

The coverage report will be generated in `build/coverage_html/index.html`.

### Documentation Build

```sh
cmake --preset=docs
cmake --build build --target docs
```

Requires Doxygen to be installed. Output is in `build/docs/html/index.html`.

### Documentation Coverage

Check how much of your code is documented:

```sh
cmake --preset=docs
cmake --build build --target docs-coverage
```

Or generate an HTML report:

```sh
cmake --preset=docs
cmake --build build --target docs-coverage-html
```

Requires Python 3 and coverxygen to be installed. HTML output is in `build/doc-coverage/index.html`.

## Available Targets

- `r-type_exe` - Main executable
- `format-check` - Check code formatting with clang-format
- `format-fix` - Automatically fix code formatting
- `coverage` - Generate coverage report (dev preset only)
- `docs` - Generate documentation (docs preset only)
- `docs-coverage` - Check documentation coverage (docs preset only)
- `docs-coverage-html` - Generate documentation coverage HTML report (docs preset only)

## Testing

Run tests with CTest:

```sh
ctest --test-dir build --output-on-failure
```

## Development Tools

### Code Formatting

The project uses clang-format for code style enforcement:

```sh
cmake --build build --target format-check  # Check formatting
cmake --build build --target format-fix    # Fix formatting
```

### Static Analysis

Use clang-tidy via the dedicated preset:

```sh
cmake --preset=clang-tidy
cmake --build build
```

### Code Coverage

Generate test coverage reports:

```sh
cmake --preset=dev
cmake --build build
ctest --test-dir build
cmake --build build --target coverage
```

Requires lcov to be installed.

### Documentation Generation

Generate API documentation with Doxygen:

```sh
cmake --preset=docs
cmake --build build --target docs
```

Requires Doxygen to be installed.

### Documentation Coverage

Check documentation coverage with a summary table:

```sh
cmake --preset=docs
cmake --build build --target docs-coverage
```

Or generate an HTML report similar to code coverage:

```sh
cmake --preset=docs
cmake --build build --target docs-coverage-html
```

Requires Python 3 and coverxygen to be installed.

## Manual Build Commands

If you prefer not to use presets:

### Debug Build

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

### Release Build

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

