# Contributing

See [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md) for community guidelines.
## Easier setup
```bash
source ./aliases.sh
```
All the next commands are available in this file

## Setup

1. Install Git hooks (required):
   ```sh
   ./scripts/setup-hooks.sh
   ```

2. Install local vcpkg:
   ```bash
   git clone git@github.com:microsoft/vcpkg.git && cd ./vcpkg/ && ./bootstrap-vcpkg.sh && cd -

   ```

3. Make changes and test:
   ```sh
   cmake --build build --preset=dev-build
   ctest --preset=dev-test --output-on-failure
   ```

4. Fix formatting before pushing:
   ```sh
   cmake --build build --target format-fix
   ```

The pre-push hook will verify formatting automatically.

## Build Presets

- `dev`: Development with coverage
- `tests`: Test configuration
- `docs`: Documentation generation
- `release`: Optimized build
- `clang-tidy`: Static analysis

## Testing

```sh
cmake --preset=tests
cmake --build build/tests --preset=tests-build
ctest --preset=dev-test --output-on-failure
```

## Coverage

```sh
cmake --preset=tests
cmake --build build/tests --preset=tests-build
ctest --preset=dev-test
cmake --build build/tests --target coverage
```

Output: `build/tests/coverage_html/index.html`

## Documentation

```sh
cmake --preset=docs
cmake --build build/docs --preset=docs-build
cmake --build build/docs --target docs-run
```

Output: `build/docs/html/index.html`

### Documentation Coverage

```sh
cmake --preset=docs
cmake --build build/docs --target docs-coverage
```

HTML report:
```sh
cmake --build build/docs --target docs-coverage-html
```

Output: `build/doc-coverage/index.html`

## Code Formatting

```sh
cmake --build build --target format-check
cmake --build build --target format-fix
```
