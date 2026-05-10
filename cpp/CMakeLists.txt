cmake_minimum_required(VERSION 3.20)
project(linalg_from_scratch CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to Release if the user has not chosen a build type.
# Switch with -DCMAKE_BUILD_TYPE=Debug (-> -O0) or RelWithDebInfo (-> -O2).
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

include(FetchContent)

# -- Catch2 v3 (test framework) --------------------------------------------
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.5.4
)
FetchContent_MakeAvailable(Catch2)

# -- Eigen: prefer system install (devcontainer apt: libeigen3-dev) --------
find_package(Eigen3 3.4 QUIET NO_MODULE)
if(NOT Eigen3_FOUND)
  message(STATUS "System Eigen3 not found; fetching from GitLab")
  set(EIGEN_BUILD_DOC OFF CACHE BOOL "" FORCE)
  set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(
    Eigen3
    GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
    GIT_TAG 3.4.0
  )
  FetchContent_MakeAvailable(Eigen3)
endif()

# -- The student-written Matrix library ------------------------------------
# Declarations in include/matrix.hpp, definitions in src/matrix.cpp.
# matrix_debug.{hpp,cpp} provides dump_raw / memory_bytes — mirrors of
# python/matrix/debug.py — used by tests/test_experiments.cpp.
# - PUBLIC include path: consumers (tests, benchmark) include "matrix.hpp".
# - PRIVATE Eigen link: only src/matrix.cpp needs Eigen, so consumers don't
#   pay for its headers when they include matrix.hpp.
add_library(matrix STATIC src/matrix.cpp src/matrix_debug.cpp)
target_include_directories(matrix PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(matrix PRIVATE Eigen3::Eigen)
target_compile_features(matrix PUBLIC cxx_std_17)

# -- Tests -----------------------------------------------------------------
# Both test files compile into the single test_matrix executable; Catch2
# discovers all TEST_CASEs across both. Tag filters (e.g. [experiments],
# [basic], [matmul]) work via the binary directly; ctest -R filters by
# test name. See docs/06 for the tag table.
enable_testing()
add_executable(test_matrix tests/test_matrix.cpp tests/test_experiments.cpp)
target_link_libraries(test_matrix PRIVATE matrix Catch2::Catch2WithMain)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(test_matrix)

# -- Benchmark -------------------------------------------------------------
add_executable(benchmark bench/benchmark.cpp)
target_link_libraries(benchmark PRIVATE matrix)

# Surface the optimisation level being used so students know what they ran.
target_compile_definitions(benchmark PRIVATE
  BENCH_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
)
