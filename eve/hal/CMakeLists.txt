cmake_minimum_required(VERSION 3.22)

project(
  hal
  VERSION 0.1.0
  LANGUAGES C)

set(CMAKE_C_STANDARD 99)
option(ARM "Whether to cross compile for ARM" OFF)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")

if (ARM)
  set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
else()
endif()

add_subdirectory(configured)
add_subdirectory(src)

#set(CMAKE_C_FLAGS "-Wall -Wextra")
#set(CMAKE_C_FLAGS_DEBUG "-g")
#set(CMAKE_C_FLAGS_RELEASE "-O3")

#find_program(asciidoctor_exe NAMES asciidoctor)
#find_program(antora_exe NAMES antora)

include(Docs)
