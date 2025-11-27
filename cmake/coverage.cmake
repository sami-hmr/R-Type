# ---- Coverage Configuration ----

set(
    COVERAGE_TRACE_COMMAND
    lcov -c
    --gcov-tool ${CMAKE_SOURCE_DIR}/cmake/llvm-gcov.sh
    -o "${PROJECT_BINARY_DIR}/coverage.info"
    -d "${PROJECT_BINARY_DIR}"
    --exclude "${PROJECT_BINARY_DIR}/_deps/*"
    --exclude "${PROJECT_SOURCE_DIR}/test/*"
    --exclude "/usr/include/*"
    --exclude "/usr/lib/*"
    --exclude "*/c++/*"
    --rc lcov_branch_coverage=1
    --ignore-errors gcov,gcov,source,source,graph,graph,unused,unused,empty,empty,version,version,mismatch,mismatch,inconsistent,inconsistent,unsupported,unsupported
)

set(
    COVERAGE_HTML_COMMAND
    genhtml
    "${PROJECT_BINARY_DIR}/coverage.info"
    -o "${PROJECT_BINARY_DIR}/coverage_html"
    --rc lcov_branch_coverage=1
    --ignore-errors source,source,inconsistent,inconsistent,corrupt,corrupt,unsupported,unsupported,deprecated,deprecated,category,category
)

# ---- Coverage target ----
add_custom_target(
    coverage
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    COMMAND ${COVERAGE_TRACE_COMMAND}
    COMMAND ${CMAKE_COMMAND} -E echo "Coverage data collected in coverage.info"
    COMMAND ${COVERAGE_HTML_COMMAND}
    COMMAND ${CMAKE_COMMAND} -E echo "HTML coverage report generated in coverage_html/"
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
    COMMENT "Running tests and generating coverage report"
    VERBATIM
)
