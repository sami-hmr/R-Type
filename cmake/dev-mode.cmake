# ---- Testing with CTest ----

include(CTest)
if(BUILD_TESTING)
  add_subdirectory(test)
endif()

# ---- Coverage ----

option(ENABLE_COVERAGE "Enable coverage support" OFF)
if(ENABLE_COVERAGE)
  include(cmake/coverage.cmake)
endif()

# ---- Documentation ----

option(BUILD_DOCS "Build documentation using Doxygen" OFF)
if(BUILD_DOCS)
  include(cmake/docs.cmake)
endif()

# ---- Linting with clang-format ----

include(cmake/lint-targets.cmake)
