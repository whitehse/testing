cmake_minimum_required(VERSION 3.22)

project(
  firetools
  VERSION 0.1.0
  LANGUAGES C)

set(CMAKE_C_STANDARD 99)

option(WASM "Whether to cross compile for Web Assembly" OFF)
option(ENABLE_WARNINGS "Enable to add warnings to a target." OFF)
option(ENABLE_WARNINGS_AS_ERRORS "Enable to treat warnings as errors." OFF)
option(ENABLE_TESTING "Enable a Unit Testing build." ON)
option(ENABLE_COVERAGE "Enable a Code Coverage build." OFF)
option(ENABLE_CLANG_TIDY "Enable to add clang tidy." OFF)
option(ENABLE_SANITIZE_ADDR "Enable address sanitize." OFF)
option(ENABLE_SANITIZE_UNDEF "Enable undefined sanitize." OFF)
option(ENABLE_SANITIZE_LEAK "Enable leak sanitize (Gcc/Clang only)." OFF)
option(ENABLE_SANITIZE_THREAD "Enable thread sanitize (Gcc/Clang only)." OFF)
option(ENABLE_CLANG_FORMAT "Enable to add clang-format." ON)
option(ENABLE_CMAKE_FORMAT "Enable to add cmake-format." ON)
option(ENABLE_LTO "Enable to add Link Time Optimization." ON)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")

if (WASM)
  set(WASI_SDK_PREFIX "/opt/wasi-sdk/")
  #include(wasi-sdk-pthread)
  include(wasi-sdk)
else()
endif()

include(CPM)
if (WASM)
else()
  cpmaddpackage("gh:ThrowTheSwitch/Unity#v2.5.2")
  cpmaddpackage("gh:cofyc/argparse@1.1.0")
endif()

#cpmaddpackage("gh:zlib-ng/zlib-ng#develop")
#cpmaddpackage("gh:madler/zlib@1.3")

include(AddGitSubmodule)
#add_git_submodule(third_party/zlib)

include(Docs)

set(LIBRARY_NAME "firetools")
set(UNIT_TEST_NAME "unit_tests")

#add_subdirectory(third_party)
add_subdirectory(configured)
add_subdirectory(third_party)
add_subdirectory(src)
include(CPM)
if (WASM)
else()
  add_subdirectory(tests)
endif()

#add_test(NAME test_test COMMAND bash "[1 = 0]")
