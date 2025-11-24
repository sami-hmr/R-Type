# ---- Variables ----

# Detect compiler and set appropriate coverage tools
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Use llvm-cov for Clang
    # The test executable is in CMAKE_SOURCE_DIR due to CMAKE_RUNTIME_OUTPUT_DIRECTORY setting
    set(TEST_EXECUTABLE "${CMAKE_SOURCE_DIR}/r-type_test")
    
    # Custom target with proper shell commands
    add_custom_target(
        coverage
        COMMAND ${CMAKE_COMMAND} -E echo "Searching for profraw files..."
        COMMAND find "${PROJECT_BINARY_DIR}" -name "*.profraw" -type f
        COMMAND ${CMAKE_COMMAND} -E echo "Merging profraw files..."
        COMMAND llvm-profdata merge -sparse "${PROJECT_BINARY_DIR}/test/default.profraw" -o "${PROJECT_BINARY_DIR}/coverage.profdata"
        COMMAND ${CMAKE_COMMAND} -E echo "Generating lcov format coverage..."
        COMMAND sh -c "llvm-cov export -format=lcov -instr-profile=${PROJECT_BINARY_DIR}/coverage.profdata ${TEST_EXECUTABLE} > ${PROJECT_BINARY_DIR}/coverage.info"
        COMMAND ${CMAKE_COMMAND} -E echo "Generating HTML coverage report..."
        COMMAND llvm-cov show -format=html -instr-profile="${PROJECT_BINARY_DIR}/coverage.profdata" "${TEST_EXECUTABLE}" -output-dir="${PROJECT_BINARY_DIR}/coverage_html"
        COMMAND ${CMAKE_COMMAND} -E echo "Coverage report generated successfully!"
        COMMENT "Generating coverage report"
        VERBATIM
    )
else()
    # Use lcov/gcov for GCC
    set(
        COVERAGE_TRACE_COMMAND
        lcov -c -q
        -o "${PROJECT_BINARY_DIR}/coverage.info"
        -d "${PROJECT_BINARY_DIR}"
        --include "${PROJECT_SOURCE_DIR}/src/*"
        --include "${PROJECT_SOURCE_DIR}/include/*"
        --include "${PROJECT_SOURCE_DIR}/plugins/*"
        --include "${PROJECT_SOURCE_DIR}/test/*"
        --ignore-errors source
        --ignore-errors inconsistent
        --ignore-errors empty
        --ignore-errors unused
        CACHE STRING
        "; separated command to generate a trace for the 'coverage' target"
    )

    set(
        COVERAGE_HTML_COMMAND
        genhtml --legend -f -q
        "${PROJECT_BINARY_DIR}/coverage.info"
        -p "${PROJECT_SOURCE_DIR}"
        -o "${PROJECT_BINARY_DIR}/coverage_html"
        --ignore-errors inconsistent
        --ignore-errors corrupt
        CACHE STRING
        "; separated command to generate an HTML report for the 'coverage' target"
    )

    # ---- Coverage target ----
    add_custom_target(
        coverage
        COMMAND ${COVERAGE_TRACE_COMMAND}
        COMMAND ${COVERAGE_HTML_COMMAND}
        COMMENT "Generating coverage report"
        VERBATIM
    )
endif()
