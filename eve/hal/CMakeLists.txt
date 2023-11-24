cmake_minimum_required(VERSION 3.22)

project(
  hal
  VERSION 0.1.0
  LANGUAGES C)

set(CMAKE_C_STANDARD 99)

option(ARM "Whether to cross compile for ARM" OFF)
option(ENABLE_WARNINGS "Enable to add warnings to a target." ON)
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

if (ARM)
  set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
else()
endif()

include(CPM)
cpmaddpackage("gh:ThrowTheSwitch/Unity#v2.5.2")
cpmaddpackage("gh:cofyc/argparse@1.1.0")

if(ENABLE_WARNINGS)
    include(Warnings)
endif()

set(LIBRARY_NAME "hal")
set(UNIT_TEST_NAME "unit_tests")

add_subdirectory(configured)
add_subdirectory(src)
add_subdirectory(tests)

#set(CMAKE_C_FLAGS "-Wall -Wextra")
#set(CMAKE_C_FLAGS_DEBUG "-g")
#set(CMAKE_C_FLAGS_RELEASE "-O3")

#find_program(asciidoctor_exe NAMES asciidoctor)
#find_program(antora_exe NAMES antora)

include(Docs)
