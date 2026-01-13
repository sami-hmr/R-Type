cmake_minimum_required(VERSION 3.14)

set(LIB_NAME ui)

add_library(${LIB_NAME} SHARED
  src/UI.cpp
  src/CameraEvents.cpp
    src/AnimatedSprite.cpp

)

target_compile_features(${LIB_NAME} PRIVATE cxx_std_23)

# MSVC needs /bigobj for large translation units
if(MSVC)
    target_compile_options(${LIB_NAME} PRIVATE /bigobj)
endif()

target_link_libraries(${LIB_NAME}
    ${CORE_LIB}
    Vector2D
)

target_include_directories(${LIB_NAME}
  PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    include
)
