# Building

## Quick Start

Load aliases from `aliases.sh` for convenient commands:

```sh
source aliases.sh
dev-all
```

if vcpkg isn't setup:

```bash
git clone git@github.com:microsoft/vcpkg.git && cd ./vcpkg/ && ./bootstrap-vcpkg.sh && cd -
```

After sourcing aliases, use CMake commands directly as shown below.

## Requirements

- CMake 3.14+
- C++23 compiler (GCC 12+, Clang 15+)
- Git

### Optional Dependencies

- lcov, perl-gd: Coverage reports
- Python 3, coverxygen: Documentation coverage
- Doxygen: API documentation

### Installation

**Debian/Ubuntu:**
```sh
sudo apt install cmake g++ git lcov libgd-perl doxygen
pip3 install coverxygen
```

**Arch Linux:**
```sh
sudo pacman -S cmake gcc git lcov perl-gd doxygen
pip3 install coverxygen
```

**macOS:**
```sh
brew install cmake git lcov doxygen
cpan install GD
pip3 install coverxygen
```

## Build Configurations

### Development

```sh
cmake --preset=dev
cmake --build build --preset=dev-build
```

### Release

```sh
cmake --preset=release
cmake --build build/release --preset=release-build
```

### Tests

```sh
cmake --preset=tests
cmake --build build/tests --preset=tests-build
ctest --preset=dev-test
```

### Documentation

```sh
cmake --preset=docs
cmake --build build/docs --preset=docs-build
cmake --build build/docs --target docs-run
```

## Available Targets

- `format-check`: Verify code formatting
- `format-fix`: Auto-fix code formatting
- `coverage`: Generate test coverage report (tests preset)
- `docs`: Generate API documentation (docs preset)
- `docs-coverage`: Documentation coverage summary (docs preset)
- `docs-coverage-html`: Documentation coverage HTML report (docs preset)

## Code Formatting

```sh
cmake --build build --target format-check
cmake --build build --target format-fix
```

## Coverage Reports

```sh
cmake --preset=tests
cmake --build build/tests --preset=tests-build
ctest --preset=dev-test
cmake --build build/tests --target coverage
```

Report output: `build/tests/coverage_html/index.html`

## Documentation Coverage

```sh
cmake --preset=docs
cmake --build build/docs --target docs-coverage
```

HTML report:
```sh
cmake --build build/docs --target docs-coverage-html
```

Output: `build/doc-coverage/index.html`
