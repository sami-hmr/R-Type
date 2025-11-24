# Hacking

Developer notes for building and contributing to this project.

For contribution guidelines, see [CONTRIBUTING.md](CONTRIBUTING.md).

## Quick Start

```sh
cmake --preset=dev
cmake --build build
./r-type
```

### Development Workflow

```sh
# Make changes
vim src/main.cpp

# Rebuild
cmake --build build

# Run
./r-type

# Run tests
ctest --test-dir build --output-on-failure

# Check formatting
cmake --build build --target format-check
```

## Developer Mode

The `r-type_DEVELOPER_MODE` option enables development-specific targets like tests, coverage, and documentation. It's enabled by default when using the `dev` preset.

Enable it manually:
```sh
cmake -B build -Dr-type_DEVELOPER_MODE=ON
```

Developer mode is always enabled in CI workflows.

## CMake Presets

Available presets for different workflows:

- `dev` - Development with tests and coverage
- `docs` - Documentation generation
- `release` - Optimized production build
- `clang-tidy` - Static analysis
- `user-dev` - Custom user preset (see CMakeUserPresets.json)

Use them with:
```sh
cmake --preset=<name>
cmake --build build  # or appropriate build dir
```

You can customize builds by editing `CMakeUserPresets.json`.

## Development Targets

Build specific targets with `-t` or `--target`:

### format-check

Check code formatting:
```sh
cmake --build build --target format-check
```

### format-fix

Fix code formatting:
```sh
cmake --build build --target format-fix
```

### coverage

Generate coverage report (dev preset includes coverage):
```sh
cmake --preset=dev
cmake --build build
ctest --test-dir build
cmake --build build --target coverage
```

Output: `build/coverage_html/index.html`

### docs

Generate documentation (requires docs preset):
```sh
cmake --preset=docs
cmake --build build --target docs
```

Output: `build/docs/html/index.html`

### docs-coverage

Check documentation coverage with a summary table:
```sh
cmake --preset=docs
cmake --build build --target docs-coverage
```

### docs-coverage-html

Generate HTML documentation coverage report:
```sh
cmake --preset=docs
cmake --build build --target docs-coverage-html
```

Output: `build/doc-coverage/index.html`

## Testing

Run tests with CTest:
```sh
ctest --test-dir build --output-on-failure
```

Run specific tests:
```sh
ctest --test-dir build -R test_name
```

## Code Quality

### Formatting

The project uses clang-format with the configuration in `.clang-format`. Use the CMake targets to check and fix formatting.

### Static Analysis

Run clang-tidy analysis:
```sh
cmake --preset=clang-tidy
cmake --build build
```

### Coverage

Track test coverage:
```sh
cmake --preset=dev
cmake --build build
ctest --test-dir build
cmake --build build --target coverage
```

## Debugging

### GDB

```sh
gdb ./r-type
```

### Valgrind

Check for memory leaks:
```sh
valgrind --leak-check=full ./r-type
```

### Core Dumps

Debug a core dump:
```sh
gdb ./r-type core
```

## Development Environment

Source the activate.sh script for convenient aliases:
```sh
source activate.sh
show-aliases
```

This provides shortcuts like:
- `dev-build` - Quick build
- `dev-test` - Run tests
- `qbr` - Quick build and run
- `coverage` - Generate coverage
- `docs` - Generate docs
- `docs-coverage` - Check documentation coverage
- `docs-coverage-html` - Generate documentation coverage report

```

## Tips

- Use `compile_commands.json` for IDE integration (auto-generated in build/)
- Run `format-fix` before committing
- Check coverage when adding tests
- Update docs when changing public APIs
- Use `docs-coverage` to ensure proper documentation
