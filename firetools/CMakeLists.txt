cmake_minimum_required(VERSION 3.22)

project(
  firetools
  VERSION 0.1.0
  LANGUAGES C)

set(CMAKE_C_STANDARD 99)
option(WASM "Whether to cross compile for Web Assembly" OFF)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")

if (WASM)
  set(WASI_SDK_PREFIX "/opt/wasi-sdk/")
  #include(wasi-sdk-pthread)
  include(wasi-sdk)
else()
endif()


#include(CPM)
#cpmaddpackage("gh:zlib-ng/zlib-ng#develop")
#cpmaddpackage("gh:madler/zlib@1.3")

include(AddGitSubmodule)
#add_git_submodule(third_party/zlib)

include(Docs)

#add_subdirectory(third_party)
add_subdirectory(configured)
add_subdirectory(src)
