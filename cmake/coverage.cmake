# ---- Coverage Configuration ----

# Detect compiler and set appropriate coverage tools
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Use llvm-cov for Clang
    # The test executable is in CMAKE_SOURCE_DIR due to CMAKE_RUNTIME_OUTPUT_DIRECTORY setting
    set(TEST_EXECUTABLE "${CMAKE_SOURCE_DIR}/r-type_test")
    
    # Custom target that calls the coverage script
    add_custom_target(
        coverage
        COMMAND bash "${CMAKE_SOURCE_DIR}/cmake/generate_coverage.sh" "${PROJECT_BINARY_DIR}" "${TEST_EXECUTABLE}"
        WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
        COMMENT "Generating coverage report"
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
        --exclude "${PROJECT_BINARY_DIR}/_deps/*"
        --exclude "${PROJECT_SOURCE_DIR}/test/*"
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
