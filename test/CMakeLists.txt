# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(r-typeTests LANGUAGES CXX)

# ---- Dependencies ----

include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0
)

FetchContent_MakeAvailable(Catch2)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)

# ---- Tests ----

add_executable(r-type_test source/r-type_test.cpp)
target_link_libraries(
    r-type_test PRIVATE
    Catch2::Catch2WithMain
)
target_compile_features(r-type_test PRIVATE cxx_std_23)

catch_discover_tests(r-type_test)
