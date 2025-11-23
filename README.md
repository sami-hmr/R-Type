# r-type

An ECS (Entity Component System) game engine implementation.

## Building the Project

### Quick Start

```bash
cmake --preset=dev
cmake --build build
./r-type
```

### Available Presets

- `dev` - Development build with tests and coverage enabled
- `docs` - Generate documentation
- `release` - Optimized release build
- `clang-tidy` - Development build with static analysis

### Building Specific Targets

```bash
# Build only the executable
cmake --build build --target r-type_exe

# Check code formatting
cmake --build build --target format-check

# Fix code formatting
cmake --build build --target format-fix
```

## Running Tests

```bash
ctest --test-dir build --output-on-failure
```

## Generating Documentation

```bash
cmake --preset=docs
cmake --build build --target docs
# Open build/docs/html/index.html
```

## Documentation Coverage

Check how much of your code is documented:

```bash
cmake --preset=docs
cmake --build build --target docs-coverage
```

Or generate an HTML report:

```bash
cmake --preset=docs
cmake --build build --target docs-coverage-html
# Open build/doc-coverage/index.html
```

## Coverage Reports

```bash
cmake --preset=dev
cmake --build build
ctest --test-dir build
cmake --build build --target coverage
# Open build/coverage_html/index.html
```

## Development Environment

Source the activate.sh script for convenient aliases:

```bash
source activate.sh
show-aliases  # Display available commands
```

For detailed build instructions, see [BUILDING.md](BUILDING.md).

For development guidelines, see [HACKING.md](HACKING.md).

## License

[MIT License](LICENSE)

