cmake_minimum_required(VERSION 3.10)

# C++ Standard is inherited from parent CMakeLists.txt
# To change the standard, modify the root CMakeLists.txt file
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fPIC -O3")

include_directories(${parser_SOURCE_DIR}/include)

add_subdirectory(moving)
