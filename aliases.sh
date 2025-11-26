alias dev-config='cmake --preset=dev'
alias dev-build='cmake --build build --preset=dev-build -j'
alias dev-all='dev-config && dev-build'

alias docs-config='cmake --preset=docs'
alias docs-build='cmake --build build/docs --preset=docs-build -j'
alias docs-run='cmake --build build/docs --target docs-run'
alias docs-all='docs-config && docs-build && docs-run'

alias docs-coverage='docs-all && cmake --build build/docs --target docs-coverage'
alias docs-coverage-html='docs-all && cmake --build build/docs --target docs-coverage-html'

tidy-fix() {
    echo "Running clang-tidy --fix-errors on all C++ files (excluding build directory)..."
    find . -type d -name build -prune -o \( -name "*.cpp" -o -name "*.hpp" \) -type f -print0 |
        xargs -0 -I {} sh -c 'echo "Processing: {}" && clang-tidy --fix-errors -p build "{}"'
    echo "✓ clang-tidy --fix-errors completed"
}

alias tests-config='cmake --preset=tests'
alias tests-build='cmake --build build/tests --preset=tests-build -j'
alias tests-run='ctest --preset=dev-test'
alias tests-all='tests-config && tests-build && tests-run'

alias coverage='tests-all && cmake --build build/tests --target coverage && xdg-open build/tests/coverage_html/index.html'

alias rel-config='cmake --preset=release'
alias rel-build='cmake --build build/release --preset=release-build -j'
alias rel-all='rel-config && rel-build'

alias format-check='cmake --build build --target format-check'
alias format-fix='cmake --build build --target format-fix'
