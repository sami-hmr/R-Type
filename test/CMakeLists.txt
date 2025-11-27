
cmake_minimum_required(VERSION 3.14)
project(r-typeTests LANGUAGES CXX)

# ---- Dependencies ----

find_package(Catch2 3 REQUIRED)

include(Catch)

# ---- Tests ----

add_executable(r-type_test source/r-type_test.cpp)
target_link_libraries(
    r-type_test PRIVATE
    r-type_core
    Catch2::Catch2WithMain
)
target_compile_features(r-type_test PRIVATE cxx_std_23)
target_include_directories(r-type_test PRIVATE ${CMAKE_SOURCE_DIR}/include)

catch_discover_tests(r-type_test)
