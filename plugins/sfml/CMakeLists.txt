cmake_minimum_required(VERSION 3.14)

set(LIB_NAME sfml)

find_package(SFML 3 COMPONENTS Graphics Window REQUIRED)

add_library(${LIB_NAME} SHARED
    src/SFMLRenderer.cpp
    src/Camera.cpp
    src/Background.cpp
)

target_compile_features(${LIB_NAME} PRIVATE cxx_std_23)

target_include_directories(${LIB_NAME}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${parser_SOURCE_DIR}/include
        include
)

target_link_libraries(${LIB_NAME} PRIVATE
    SFML::Graphics
    SFML::Window
    ${CORE_LIB}
    Vector2D
)
