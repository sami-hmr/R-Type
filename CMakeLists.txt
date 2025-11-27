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
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fPIC -O3")
set(CMAKE_SHARED_LIBRARY_PREFIX "")

# ---- Developer mode variable ----

include(FetchContent)

FetchContent_Declare(
    parser
  GIT_REPOSITORY https://github.com/abeldaverio/cpp-parsing-library
  GIT_TAG        main
)

FetchContent_MakeAvailable(parser)

# ---- Declare executable ----

set(CORE_LIB r-type_core)

add_library(${CORE_LIB} STATIC
    src/plugins/APlugin.cpp
    src/plugins/EntityLoader.cpp
)

target_include_directories(${CORE_LIB}
    PUBLIC
        include
        ${parser_SOURCE_DIR}/include
)

target_link_libraries(${CORE_LIB} PRIVATE parser)

add_executable(r-type_exe
    src/main.cpp
)
add_executable(r-type::exe ALIAS r-type_exe)

set_property(TARGET r-type_exe PROPERTY OUTPUT_NAME r-type)

target_compile_features(r-type_exe PRIVATE cxx_std_23)

target_include_directories(r-type_exe
    PRIVATE
        include
        ${parser_SOURCE_DIR}/include
)

target_link_libraries(r-type_exe PUBLIC ${CORE_LIB})

# Fix linking issue with clang
#if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
#    target_link_libraries(r-type_exe PRIVATE stdc++)
#endif()

# ---- Developer mode ----

if(r-type_DEVELOPER_MODE)
    include(cmake/dev-mode.cmake)
endif()

add_subdirectory(plugins)
