cmake_minimum_required(VERSION 3.14)

set(LIB_NAME collision)

add_library(${LIB_NAME} SHARED
    src/Collision.cpp
    src/QuadTreeCollision.cpp
)

target_compile_features(${LIB_NAME} PRIVATE cxx_std_23)

target_include_directories(${LIB_NAME}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/plugins/logger/include
        include
)
