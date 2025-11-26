# ---- Coverage Configuration ----

set(
    COVERAGE_TRACE_COMMAND
    lcov -c -q
    -o "${PROJECT_SOURCE_DIR}/coverage.info"
    -d "${PROJECT_BINARY_DIR}"
    --exclude "${PROJECT_BINARY_DIR}/_deps/*"
    --exclude "${PROJECT_SOURCE_DIR}/test/*"
    --ignore-errors empty,empty,unused,unused
)

# ---- Coverage target ----
add_custom_target(
    coverage
    COMMAND ${COVERAGE_TRACE_COMMAND}
    COMMENT "Generating coverage report"
    VERBATIM
)
