cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fPIC -O3")

include_directories(${parser_SOURCE_DIR}/include)

add_subdirectory(moving)
