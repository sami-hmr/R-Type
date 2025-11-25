cmake_minimum_required(VERSION 3.14)

set(LIB_NAME logger)

add_library(${LIB_NAME} SHARED
    src/Logger.cpp
)

target_compile_features(${LIB_NAME} PRIVATE cxx_std_23)

target_include_directories(${LIB_NAME}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        include
)
