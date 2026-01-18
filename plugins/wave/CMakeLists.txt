cmake_minimum_required(VERSION 3.14)

set(LIB_NAME wave)

add_library(${LIB_NAME} SHARED
    src/WaveManager.cpp
    src/WaveFormation.cpp
    src/WavePattern.cpp
    src/WaveSpawner.cpp
    src/WaveDeath.cpp
)

target_compile_features(${LIB_NAME} PRIVATE cxx_std_23)

target_link_libraries(${LIB_NAME} ${CORE_LIB} Vector2D)

target_include_directories(${LIB_NAME}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        include
)
