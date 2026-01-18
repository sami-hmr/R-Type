cmake_minimum_required(VERSION 3.14)

set(LIB_NAME rtype_single)

add_library(${LIB_NAME} SHARED
    src/RtypeSingle.cpp
)

target_compile_features(${LIB_NAME} PRIVATE cxx_std_23)

target_link_libraries(${LIB_NAME}
    PRIVATE
    ${CLIENT_LIB}
)

target_include_directories(${LIB_NAME}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        include
)
