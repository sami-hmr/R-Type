cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    r-type
    VERSION 0.1.0
    DESCRIPTION "ecs game engine"
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/plugins)
set(CMAKE_SHARED_LIBRARY_PREFIX "")

# ---- Developer mode variable ----

if(PROJECT_IS_TOP_LEVEL)
    option(r-type_DEVELOPER_MODE "Enable developer mode" OFF)
endif()

include(FetchContent)

FetchContent_Declare(
    parser
  GIT_REPOSITORY https://github.com/abeldaverio/cpp-parsing-library
  GIT_TAG        main
)

FetchContent_MakeAvailable(parser)

add_subdirectory(plugins)

# ---- Declare executable ----

add_executable(r-type_exe src/main.cpp)
add_executable(r-type::exe ALIAS r-type_exe)

set_property(TARGET r-type_exe PROPERTY OUTPUT_NAME r-type)

target_compile_features(r-type_exe PRIVATE cxx_std_23)

target_include_directories(r-type_exe
    PRIVATE
        include
        ${parser_SOURCE_DIR}/include
)

target_link_libraries(r-type_exe PRIVATE parser)



# Fix linking issue with clang
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_link_libraries(r-type_exe PRIVATE stdc++)
endif()

# ---- Documentation ----

option(BUILD_DOCS "Build documentation using Doxygen" OFF)

if(BUILD_DOCS)
    include(cmake/docs.cmake)
endif()

# ---- Developer mode ----

if(r-type_DEVELOPER_MODE)
    include(cmake/dev-mode.cmake)
endif()
