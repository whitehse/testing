set(LIBRARY_NAME "spreadsheetmlrw")

set(LIBRARY_SOURCES
  "spreadsheet.c"
  "calcsheet.c"
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
  "${CMAKE_BINARY_DIR}/configured_files/include"
  "${CMAKE_BINARY_DIR}/third_party/minizip-ng/"
)
  #"${PROJECT_SOURCE_DIR}/third_party/minizip-ng/"

add_library(${LIBRARY_NAME} STATIC
  "${LIBRARY_SOURCES}"
)
  #"${LIBRARY_HEADERS"

add_executable(ssmlexe
  "${EXE_SOURCES}"
)

#if (WASM)
#else()
#endif()

#target_link_directories(exe PUBLIC "${CMAKE_BINARY_DIR}/third_party/minizip-ng")
#target_link_libraries(ssmlexe m)
target_link_libraries(ssmlexe minizip)

#target_link_options(main PRIVATE "LINKER:--no-check-features")

#gperf -D -s 10 -t spreadsheetml_types.gperf > spreadsheetml_types.c
#gcc -I. -o test spreadsheetml_types.c main.c
