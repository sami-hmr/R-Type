# ---- Dependencies ----

find_package(Doxygen REQUIRED)

# ---- Declare documentation target ----

set(
    DOXYGEN_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/docs"
    CACHE PATH "Path for the generated Doxygen documentation"
)

set(working_dir "${PROJECT_BINARY_DIR}/docs")

configure_file("docs/Doxyfile.in" "${working_dir}/Doxyfile" @ONLY)

add_custom_target(
    docs
    COMMAND "${CMAKE_COMMAND}" -E remove_directory
    "${DOXYGEN_OUTPUT_DIRECTORY}/html"
    "${DOXYGEN_OUTPUT_DIRECTORY}/xml"
    COMMAND "${DOXYGEN_EXECUTABLE}" "${working_dir}/Doxyfile"
    COMMENT "Building documentation using Doxygen"
    WORKING_DIRECTORY "${working_dir}"
    VERBATIM
)

# ---- Documentation coverage target ----

find_program(PYTHON3_EXECUTABLE python3)

if(PYTHON3_EXECUTABLE)
    add_custom_target(
        docs-coverage
        COMMAND "${PYTHON3_EXECUTABLE}" -m coverxygen
        --xml-dir "${DOXYGEN_OUTPUT_DIRECTORY}/xml"
        --src-dir "${PROJECT_SOURCE_DIR}"
        --output -
        --format summary
        --exclude ".*/pages/.*"
        --exclude ".*\\.md$"
        COMMENT "Generating documentation coverage report"
        DEPENDS docs
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        VERBATIM
    )

    add_custom_target(
        docs-coverage-html
        COMMAND "${PYTHON3_EXECUTABLE}" -m coverxygen
        --xml-dir "${DOXYGEN_OUTPUT_DIRECTORY}/xml"
        --src-dir "${PROJECT_SOURCE_DIR}"
        --output "${PROJECT_BINARY_DIR}/doc-coverage.info"
        --format lcov
        --exclude ".*/pages/.*"
        --exclude ".*\\.md$"
        COMMAND genhtml --no-function-coverage --no-branch-coverage
        --ignore-errors source
        --filter missing
        "${PROJECT_BINARY_DIR}/doc-coverage.info"
        -o "${PROJECT_BINARY_DIR}/doc-coverage"
        COMMENT "Generating HTML documentation coverage report"
        DEPENDS docs
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        VERBATIM
    )
else()
    message(WARNING "Python3 not found, documentation coverage targets will not be available")
endif()
