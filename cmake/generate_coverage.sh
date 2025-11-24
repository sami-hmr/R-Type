#!/bin/bash
set -e

PROJECT_BINARY_DIR="${1:-.}"
TEST_EXECUTABLE="${2:-./r-type_test}"

cd "$PROJECT_BINARY_DIR"

echo "Searching for profraw files..."
find . -name "*.profraw" -type f

profraw_count=$(find . -name "*.profraw" -type f | wc -l)
if [ "$profraw_count" -eq 0 ]; then
    echo "error: no profraw files found"
    exit 1
fi

echo "Found $profraw_count profraw file(s)"
echo "Merging profraw files..."

# Collect all profraw files
profraw_files=$(find . -name "*.profraw" -type f)

# Merge using llvm-profdata
llvm-profdata merge -sparse $profraw_files -o coverage.profdata

echo "Checking if coverage.profdata was created..."
if [ ! -f coverage.profdata ]; then
    echo "error: failed to create coverage.profdata"
    exit 1
fi
echo "coverage.profdata exists"

echo "Generating lcov format coverage..."
llvm-cov export -format=lcov -instr-profile=coverage.profdata "$TEST_EXECUTABLE" \
    -ignore-filename-regex='.*/_deps/.*' \
    -ignore-filename-regex='.*/test/.*' > coverage.info

echo "Generating HTML coverage report..."
llvm-cov show -format=html -instr-profile=coverage.profdata "$TEST_EXECUTABLE" \
    -ignore-filename-regex='.*/_deps/.*' \
    -ignore-filename-regex='.*/test/.*' \
    -output-dir=coverage_html

echo "Coverage report generated successfully!"
