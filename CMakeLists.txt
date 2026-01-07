cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    r-type
    VERSION 0.1.0
    DESCRIPTION "ecs game engine"
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fPIC -O3")
set(CMAKE_SHARED_LIBRARY_PREFIX "")

# Use vcpkg toolchain if available
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

# ---- Developer mode variable ----

include(FetchContent)

FetchContent_Declare(
    parser
  GIT_REPOSITORY https://github.com/abeldaverio/cpp-parsing-library
  GIT_TAG        main
)

FetchContent_MakeAvailable(parser)

# ---- Declare executable ----

set(CORE_LIB r-type_core)

add_library(${CORE_LIB} STATIC
    src/plugins/APlugin.cpp
    src/plugins/EntityLoader.cpp
    src/plugins/Byte.cpp
    src/Registry.cpp
    src/EventManager.cpp
)

target_include_directories(${CORE_LIB}
    PUBLIC
        include
        ${parser_SOURCE_DIR}/include
)

target_link_libraries(${CORE_LIB} PRIVATE parser)

find_package(asio CONFIG REQUIRED)
find_package(ZLIB REQUIRED)

set(SERVER_LIB r-type_server_lib)

add_library(${SERVER_LIB} STATIC
    src/network/server/true_server/ClientManagement.cpp
    src/network/server/true_server/CommandParsing.cpp
    src/network/server/true_server/Connected.cpp
    src/network/server/true_server/Connectionless.cpp
    src/network/server/true_server/DataTransfer.cpp
    src/network/server/true_server/Server.cpp
    src/network/server/plugin_base/BaseServer.cpp
    src/network/AcknowledgeManager.cpp
    src/network/PacketCompresser.cpp
)

target_include_directories(${SERVER_LIB}
    PUBLIC
        include
        ${parser_SOURCE_DIR}/include
)

target_link_libraries(${SERVER_LIB} PRIVATE
    parser
    ${CORE_LIB}
    asio::asio
    ZLIB::ZLIB
)

set(CLIENT_LIB r-type_client_lib)

add_library(${CLIENT_LIB} STATIC
    src/network/client/true_client/CommandParsing.cpp
    src/network/client/true_client/Connected.cpp
    src/network/client/true_client/Connectionless.cpp
    src/network/client/true_client/DataTransfer.cpp
    src/network/client/true_client/Client.cpp
    src/network/client/plugin_base/BaseClient.cpp
    src/network/AcknowledgeManager.cpp
    src/network/PacketCompresser.cpp
)

target_include_directories(${CLIENT_LIB}
    PUBLIC
        include
        ${parser_SOURCE_DIR}/include
)

target_link_libraries(${CLIENT_LIB} PRIVATE
    parser
    ${CORE_LIB}
    asio::asio
    ZLIB::ZLIB
)



if(BUILD_EPITECH_BINARIES)
    add_executable(r-type_client
        src/main.cpp
    )

    set_property(TARGET r-type_client PROPERTY OUTPUT_NAME r-type_client)

    target_compile_features(r-type_client PRIVATE cxx_std_23)

    target_compile_definitions(r-type_client PRIVATE RTYPE_EPITECH_CLIENT)

    target_include_directories(r-type_client
        PRIVATE
            include
            ${parser_SOURCE_DIR}/include
    )

    target_link_libraries(r-type_client PUBLIC ${CORE_LIB})

    add_executable(r-type_server
        src/main.cpp
    )

    set_property(TARGET r-type_server PROPERTY OUTPUT_NAME r-type_server)

    target_compile_features(r-type_server PRIVATE cxx_std_23)

    target_compile_definitions(r-type_server PRIVATE RTYPE_EPITECH_SERVER)

    target_include_directories(r-type_server
        PRIVATE
            include
            ${parser_SOURCE_DIR}/include
    )

    target_link_libraries(r-type_server PUBLIC ${CORE_LIB})

    add_executable(r-type::exe ALIAS r-type_client)
else()
    add_executable(r-type_exe
        src/main.cpp
    )
    add_executable(r-type::exe ALIAS r-type_exe)

    set_property(TARGET r-type_exe PROPERTY OUTPUT_NAME r-type)

    target_compile_features(r-type_exe PRIVATE cxx_std_23)

    target_include_directories(r-type_exe
        PRIVATE
            include
            ${parser_SOURCE_DIR}/include
    )

    target_link_libraries(r-type_exe PUBLIC ${CORE_LIB})
endif()



# Fix linking issue with clang
#if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
#    target_link_libraries(r-type_exe PRIVATE stdc++)
#endif()

# ---- Developer mode ----

if(r-type_DEVELOPER_MODE)
    include(cmake/dev-mode.cmake)
endif()

add_subdirectory(plugins)
add_subdirectory(lib)
