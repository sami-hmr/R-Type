# ---- Coverage Configuration ----

add_custom_target(
    coverage
    COMMAND bash "${CMAKE_SOURCE_DIR}/cmake/generate_coverage.sh" "${PROJECT_BINARY_DIR}" "./build/tests/test/r-type_test"
    WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
    COMMENT "Generating coverage report"
)
