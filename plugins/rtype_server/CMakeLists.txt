cmake_minimum_required(VERSION 3.14)

set(LIB_NAME rtype_server)

add_library(${LIB_NAME} SHARED
    src/RtypeServer.cpp
    src/HttpRequests.cpp
)

target_compile_features(${LIB_NAME} PRIVATE cxx_std_23)

target_link_libraries(${LIB_NAME}
    PRIVATE
    ${SERVER_LIB}
)

target_include_directories(${LIB_NAME}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        include
)
