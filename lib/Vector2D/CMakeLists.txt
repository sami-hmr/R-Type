cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set(LIB_NAME Vector2D)

add_library(${LIB_NAME} STATIC
    src/Vector2D.cpp
)

target_include_directories(${LIB_NAME}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include/
    ${parser_SOURCE_DIR}/include
)

target_link_libraries(${LIB_NAME} PRIVATE parser)

# Set compiler-specific flags using target properties instead of CMAKE_CXX_FLAGS
if(MSVC)
  target_compile_options(${LIB_NAME} PRIVATE /W4 /O2)
  # Remove any GCC/Clang flags that might have been inherited
  get_target_property(CURRENT_FLAGS ${LIB_NAME} COMPILE_OPTIONS)
  if(CURRENT_FLAGS)
    list(FILTER CURRENT_FLAGS EXCLUDE REGEX "^-")
    set_target_properties(${LIB_NAME} PROPERTIES COMPILE_OPTIONS "${CURRENT_FLAGS}")
  endif()
else()
  target_compile_options(${LIB_NAME} PRIVATE -Wall -Wextra -fPIC -O3)
endif()

