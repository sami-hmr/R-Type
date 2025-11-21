cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)
include(cmake/variables.cmake)

project(
    r-type
    VERSION 0.1.0
    DESCRIPTION "esc game engine"
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)

# ---- Declare library ----

add_library(
    r-type_lib OBJECT
    source/lib.cpp
)

target_include_directories(
    r-type_lib ${warning_guard}
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_compile_features(r-type_lib PUBLIC cxx_std_17)

find_package(fmt REQUIRED)
target_link_libraries(r-type_lib PRIVATE fmt::fmt)

# ---- Declare executable ----

add_executable(r-type_exe source/main.cpp)
add_executable(r-type::exe ALIAS r-type_exe)

set_property(TARGET r-type_exe PROPERTY OUTPUT_NAME r-type)

target_compile_features(r-type_exe PRIVATE cxx_std_17)

target_link_libraries(r-type_exe PRIVATE r-type_lib)

# Fix linking issue with clang
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_link_libraries(r-type_exe PRIVATE stdc++)
endif()

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
    include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT r-type_DEVELOPER_MODE)
    return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
    message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of r-type"
  )
endif()

include(cmake/dev-mode.cmake)
