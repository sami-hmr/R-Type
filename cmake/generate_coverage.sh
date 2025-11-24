#!/bin/bash
set -e

PROJECT_BINARY_DIR="${1:-.}"
TEST_EXECUTABLE="${2:-./r-type_test}"

cd "$PROJECT_BINARY_DIR"

echo "Searching for profraw files..."
profraw_files=$(find . -name "*.profraw" -type f 2>/dev/null)

if [ -z "$profraw_files" ]; then
    echo "error: no profraw files found"
    echo "This usually means tests didn't run or weren't instrumented with coverage flags"
    exit 1
fi

echo "Found profraw files:"
echo "$profraw_files"

profraw_count=$(echo "$profraw_files" | wc -l)
echo "Found $profraw_count profraw file(s)"

echo "Merging profraw files..."
# Merge using llvm-profdata with explicit file list
echo "$profraw_files" | xargs llvm-profdata merge -sparse -o coverage.profdata

if [ ! -f coverage.profdata ]; then
    echo "error: failed to create coverage.profdata"
    exit 1
fi

echo "Checking if coverage.profdata was created..."
ls -lh coverage.profdata
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
