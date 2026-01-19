# R-Type

A modern, plugin-based game engine built with Entity Component System (ECS) architecture. Originally inspired by the classic R-Type shoot 'em up game, this engine provides a flexible, high-performance foundation for building networked multiplayer games.

## Features

- **ECS Architecture**: Pure Entity Component System design for maximum flexibility and performance
- **Plugin System**: Modular plugin architecture for easy extensibility
- **Networked Multiplayer**: Built-in client-server networking with ASIO
- **JSON Configuration**: Define entities, components, and game logic via JSON templates
- **SFML Integration**: Graphics and audio rendering through SFML 3.0
- **Event System**: Robust event-driven architecture for decoupled game logic
- **Cross-Platform**: Supports Linux, and Windows

## Getting Started

### Prerequisites

Before building the project, ensure you have the following installed:

- **CMake** 3.14 or higher
- **C++23 compatible compiler**:
  - GCC 12+ (Linux)
  - Clang 15+ (macOS)
  - MSVC 2022+ (Windows)
- **Git**
- **vcpkg** (for dependency management)

### Quick Start

1. **Clone the repository**:
   ```bash
   git clone https://github.com/sami-hmr/R-Type.git
   cd rtype
   ```

2. **Set up vcpkg** (if not already installed):
   ```bash
   # Using the provided alias after sourcing aliases.sh
   source aliases.sh
   vcpkg-init
   
   # Or manually:
   git clone https://github.com/microsoft/vcpkg.git
   cd vcpkg
   ./bootstrap-vcpkg.sh  # On Windows: .\bootstrap-vcpkg.bat
   cd ..
   ```

3. **Build and run**:
   ```bash
   cmake --preset=release
   cmake --build build/release --preset=release-build
   
   # Run with a configuration directory
   ./build/release/r-type client_config
   ```

### Running the Game

The R-Type binary loads JSON configuration directories that define scenes, plugins, entities, and game logic.

**Using Configuration Directories**:
```bash
# Run client configuration
./build/release/r-type client_config

# Run server configuration  
./build/release/r-type server_config

# You can also load multiple config directories
./build/release/r-type client_config game_config
```

**Available Configurations**:
- `client_config/` - Client-side game scenes (menus, gameplay UI, controls)
- `server_config/` - Server-side game logic and networking
- `game_config/` - Shared game content (entities, levels, templates)

Each configuration directory contains JSON files that define:
- **Scenes**: Game states (menus, gameplay, etc.)
- **Plugins**: Which systems to load (rendering, physics, networking, etc.)
- **Entities**: Game objects with their components
- **Templates**: Reusable entity definitions

**EPITECH Release Binaries**:

For the EPITECH release, separate binaries are built that automatically load the correct configuration:
- `r-type_client` - Automatically loads `client_config/`
- `r-type_server` - Automatically loads `server_config/`

### Development Build

For active development with debug symbols and additional tooling:

```bash
# Load convenient build aliases
source aliases.sh

# Build everything in development mode
dev-all

# Run with development build
./build/r-type client_config
```

#### Available Aliases

After sourcing `aliases.sh`, you have access to these shortcuts:

**Development**:
- `dev-config` - Configure development build
- `dev-build` - Build development target
- `dev-all` - Configure and build development

**Release**:
- `rel-config` - Configure release build
- `rel-build` - Build release target
- `rel-all` - Configure and build release

**Testing**:
- `tests-config` - Configure tests
- `tests-build` - Build tests
- `tests-run` - Run test suite
- `tests-all` - Configure, build, and run tests
- `coverage` - Generate coverage report and open in browser

**Documentation**:
- `docs-config` - Configure documentation
- `docs-build` - Build documentation
- `docs-run` - Open documentation in browser
- `docs-all` - Configure, build, and open docs
- `docs-coverage` - Generate documentation coverage
- `docs-coverage-html` - Generate HTML documentation coverage report

**Code Quality**:
- `format-check` - Check code formatting
- `format-fix` - Auto-fix code formatting
- `tidy-fix` - Run clang-tidy with auto-fix

**Setup**:
- `vcpkg-init` - Clone and bootstrap vcpkg

## Building

See [BUILDING.md](BUILDING.md) for comprehensive build instructions, including:
- Platform-specific setup
- Build configurations (dev, release, tests, docs)
- Code formatting and linting
- Coverage reports
- Documentation generation

## Documentation

- **[Plugin Development Guide](docs/plugin-development.md)**: Step-by-step guide to creating plugins
- **[Component Reference](docs/COMPONENTS.md)**: All available ECS components
- **[Entity Templates](docs/entity-templates.md)**: JSON entity configuration guide
- **[Event System](docs/EVENTS.md)**: Event handling and custom events
- **[Network Protocol](docs/PROTOCOLE.md)**: Client-server communication protocol
- **[Hooks System](docs/hooks.md)**: Advanced component interaction patterns

Generate API documentation:
```bash
cmake --preset=docs
cmake --build build/docs --target docs
# Open build/docs/html/index.html
```

## Testing

Run the test suite:
```bash
cmake --preset=tests
cmake --build build/tests --preset=tests-build
ctest --preset=dev-test
```

Generate coverage report:
```bash
cmake --build build/tests --target coverage
# Open build/tests/coverage_html/index.html
```

## Contributing

We welcome contributions! Please read [CONTRIBUTING.md](CONTRIBUTING.md) for:
- Code style guidelines
- Development workflow
- Pull request process
- Code of conduct

## Plugin Development

The engine uses a plugin system for extensibility. Each plugin can register:
- Custom components
- Systems (game logic)
- Event handlers
- Entity templates

See the **[Plugin Development Guide](docs/plugin-development.md)** for a complete walkthrough of creating your own plugins.

Quick example:
```cpp
class MyPlugin : public APlugin {
public:
    MyPlugin(Registry& r, EventManager& em, EntityLoader& l)
        : APlugin("myplugin", r, em, l, {}, 
                  {COMP_INIT(MyComponent, MyComponent, init_my_component)})
    {
        REGISTER_COMPONENT(MyComponent)
        
        // Add system
        r.add_system([this](Registry& r) { this->my_system(r); }, 4);
        
        // Subscribe to events
        SUBSCRIBE_EVENT(MyEvent, {
            // Handle event
        })
    }
};
```

## Architecture

The engine follows a strict ECS pattern:
- **Entities**: Unique identifiers (just integers)
- **Components**: Pure data structures
- **Systems**: Logic that operates on components
- **Plugins**: Encapsulate related components and systems

This architecture ensures:
- High performance through cache-friendly memory layout
- Easy parallel processing
- Clear separation of concerns
- Hot-reloadable plugins (planned)

## Dependencies

Managed automatically via vcpkg:
- **SFML 3.0+**: Graphics and audio
- **ASIO 1.32+**: Asynchronous networking
- **fmt 11.0+**: String formatting
- **Catch2 3.7+**: Testing framework (dev only)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Inspired by the classic R-Type arcade game by Irem
- Built with modern C++23 features
- ECS architecture inspired by EnTT and Flecs

## Support

For questions and support:
- Check the [documentation](docs/)
- Open an [issue](https://github.com/yourusername/rtype/issues)
- Join our community discussions
