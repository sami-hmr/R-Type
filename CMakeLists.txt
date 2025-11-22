cmake_minimum_required(VERSION 3.14)

set(PROJECT_NAME rtype)

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/plugins)
set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(INCLUDES_DIRECTORY ${CMAKE_SOURCE_DIR}/include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fPIC -O3")

add_executable(${PROJECT_NAME}
    src/main.cpp
)

include(FetchContent)

FetchContent_Declare(
    parser
  GIT_REPOSITORY https://github.com/abeldaverio/cpp-parsing-library
  GIT_TAG        main
)

FetchContent_MakeAvailable(parser)

target_link_libraries(${PROJECT_NAME} PRIVATE parser)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${INCLUDES_DIRECTORY}
        ${parser_SOURCE_DIR}/include
)

add_custom_target(re
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}
    COMMENT "Rebuilding the project"
)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3")

add_custom_target(debug
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --config Debug
    COMMENT "Rebuilding the project in Debug mode"
)

add_custom_target(doc
    COMMAND mkdir -p ${CMAKE_SOURCE_DIR}/doc
    COMMAND doxygen ${CMAKE_SOURCE_DIR}/doxygen_config.txt
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating documentation with Doxygen in doc folder"
)

add_subdirectory(plugins)
