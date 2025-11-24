# ---- Variables ----

# Detect compiler and set appropriate coverage tools
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Use llvm-cov for Clang
    set(
        COVERAGE_TRACE_COMMAND
        llvm-profdata merge -sparse default.profraw -o coverage.profdata
        COMMAND llvm-cov export
        -format=lcov
        -instr-profile=coverage.profdata
        "${PROJECT_BINARY_DIR}/test/r-type_test"
        > "${PROJECT_BINARY_DIR}/coverage.info"
        CACHE STRING
        "; separated command to generate a trace for the 'coverage' target"
    )
    
    set(
        COVERAGE_HTML_COMMAND
        llvm-cov show
        -format=html
        -instr-profile=coverage.profdata
        "${PROJECT_BINARY_DIR}/test/r-type_test"
        -output-dir="${PROJECT_BINARY_DIR}/coverage_html"
        CACHE STRING
        "; separated command to generate an HTML report for the 'coverage' target"
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
endif()

# ---- Coverage target ----

add_custom_target(
    coverage
    COMMAND ${COVERAGE_TRACE_COMMAND}
    COMMAND ${COVERAGE_HTML_COMMAND}
    COMMENT "Generating coverage report"
    VERBATIM
)
