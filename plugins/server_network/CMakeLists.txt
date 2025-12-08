cmake_minimum_required(VERSION 3.14)

set(LIB_NAME server_network)

find_package(asio CONFIG REQUIRED)

add_library(${LIB_NAME} SHARED
    src/Network.cpp
    src/CommandParsing.cpp
    src/Server.cpp
    src/Connectionless.cpp
)

target_compile_features(${LIB_NAME} PRIVATE cxx_std_23)

target_link_libraries(${LIB_NAME}
    PRIVATE
    ${CORE_LIB}
    asio::asio
)

target_include_directories(${LIB_NAME}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        include
)
