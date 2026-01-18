cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Export all symbols from DLLs on Windows (needed for plugin entry points)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /O2")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fPIC -O3")
endif()

# Output plugins to build/plugins/ directory (next to executable)
# On Windows, DLLs go to RUNTIME, on Linux/Mac, .so goes to LIBRARY
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)

# Also set per-config output directories for MSVC multi-config generators
foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
  string(TOUPPER ${CONFIG} CONFIG_UPPER)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/plugins)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/plugins)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/plugins)
endforeach()

include_directories(${parser_SOURCE_DIR}/include)

# Copy required runtime DLLs on Windows
# DLLs need to be next to the executable AND next to plugin DLLs for proper loading
if(WIN32 AND DEFINED VCPKG_INSTALLED_DIR)
  # Choose the correct vcpkg bin directory based on build type
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(VCPKG_BIN_DIR "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/bin")
  else()
    set(VCPKG_BIN_DIR "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin")
  endif()
  
  file(GLOB VCPKG_DLLS "${VCPKG_BIN_DIR}/*.dll")
  foreach(DLL ${VCPKG_DLLS})
    # Copy to executable directory
    file(COPY ${DLL} DESTINATION ${CMAKE_BINARY_DIR})
    # Copy to plugins directory (for plugin DLL dependency resolution)
    file(COPY ${DLL} DESTINATION ${CMAKE_BINARY_DIR}/plugins)
  endforeach()
endif()

add_subdirectory(moving)
add_subdirectory(sfml)
add_subdirectory(logger)
add_subdirectory(life)
add_subdirectory(rtype_client)
add_subdirectory(rtype_server)
add_subdirectory(cli)
add_subdirectory(collision)
add_subdirectory(controller)
add_subdirectory(ui)
add_subdirectory(actions)
add_subdirectory(projectile)
add_subdirectory(inventory)
add_subdirectory(target)
add_subdirectory(weapon)
add_subdirectory(mob)
add_subdirectory(ath)
add_subdirectory(ai)
add_subdirectory(sound)
add_subdirectory(rtype_single)
add_subdirectory(wave)

