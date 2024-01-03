set(LIBRARY_NAME "spreadsheetmlrw")

set(LIBRARY_SOURCES
  "main.c"
  "spreadsheetml_types.c"
)

set(LIBRARY_HEADERS
  "spreadsheetmlrw.h"
)

set(EXE_SOURCES
  "main.c"
  "spreadsheetml_types.c"
)

find_package(Gperf REQUIRED)

include_directories(
  "."
  "${PROJECT_SOURCE_DIR}/include"
  "${PROJECT_SOURCE_DIR}/third_party/libcsv/"
  "${CMAKE_BINARY_DIR}/configured_files/include"
  "${CMAKE_BINARY_DIR}/third_party/minizip-ng/"
)
  #"${PROJECT_SOURCE_DIR}/third_party/minizip-ng/"

#add_library(${LIBRARY_NAME} STATIC
#  "${LIBRARY_SOURCES}"
#)
  #"${LIBRARY_HEADERS"

add_executable(ssmlexe
  "${EXE_SOURCES}"
)

#if (WASM)
#else()
#endif()

target_link_libraries(ssmlexe minizip)
target_link_libraries(ssmlexe expat)
target_link_libraries(ssmlexe csv)
#target_link_libraries(ssmlexe c)

#target_link_options(main PRIVATE "LINKER:--no-check-features")

#gperf -D -s 10 -t spreadsheetml_types.gperf > spreadsheetml_types.c
#gcc -I. -o test spreadsheetml_types.c main.c

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--export=test")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-allow-undefined-file,wasm.syms")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-entry")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--import-memory")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostartfiles")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostartfiles   -fno-exceptions  -Xlinker --defsym=__stack_size=${STACK_SIZE} -T ${LINKER_SCRIPT} -Wl,-Map=${TARGET}.map")

add_compile_definitions("NDEBUG")

#   -nostartfiles

