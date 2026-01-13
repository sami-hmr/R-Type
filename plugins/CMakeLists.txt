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

# Set output directory for all plugins to plugins/ for consistency across platforms
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# Also set per-config output directories for MSVC multi-config generators
foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
  string(TOUPPER ${CONFIG} CONFIG_UPPER)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_CURRENT_SOURCE_DIR})
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_CURRENT_SOURCE_DIR})
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_CURRENT_SOURCE_DIR})
endforeach()

include_directories(${parser_SOURCE_DIR}/include)

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
add_subdirectory(target)
add_subdirectory(weapon)
add_subdirectory(mob)
add_subdirectory(ath)
add_subdirectory(ai)
