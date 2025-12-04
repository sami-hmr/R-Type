cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fPIC -O3")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set(LIB_NAME Vector2D)

add_library(${LIB_NAME} STATIC
    src/Vector2D.cpp
)

target_include_directories(${LIB_NAME}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include/
    ${parser_SOURCE_DIR}/include
)

target_link_libraries(${LIB_NAME} PRIVATE parser)

