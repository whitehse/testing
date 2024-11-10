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
  "${PROJECT_SOURCE_DIR}/third_party/hashmap/include/"
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

target_link_libraries(ssmlexe minizip)
target_link_libraries(ssmlexe expat)
target_link_libraries(ssmlexe csv)
#target_link_libraries(ssmlexe c)

#target_link_options(main PRIVATE "LINKER:--no-check-features")

#gperf -D -s 10 -t spreadsheetml_types.gperf > spreadsheetml_types.c
#gcc -I. -o test spreadsheetml_types.c main.c

if (WASM)
  #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--export=test")
  set_target_properties(ssmlexe PROPERTIES LINK_FLAGS
    "-Wl,--export=doit -Wl,--export=malloc -Wl,--export=free -Wl,--export=get_string_buffer -Wl,--export=get_string_len -Wl,-allow-undefined-file,${PROJECT_SOURCE_DIR}/src/spreadsheetmlrw/wasm.syms -Wl,--import-memory -nostartfiles -Wl,--no-entry")
  #set_target_properties(ssmlexe PROPERTIES LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib")
  #set_target_properties(ssmlexe PROPERTIES LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostartfiles   -fno-exceptions  -Xlinker --defsym=__stack_size=${STACK_SIZE} -T ${LINKER_SCRIPT} -Wl,-Map=${TARGET}.map")
  add_compile_definitions("NDEBUG")
else()
endif()

#   -nostartfiles

#find_package(FLEX 2.6 REQUIRED)
find_package(BISON 3.0 REQUIRED)

#set(LEXER_OUT "lexer.c")
set(WORKSHEET_OUT "worksheet.c")

#flex_target(LEXER "lexer.l" "${LEXER_OUT}" DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/lexer.h")
bison_target(worksheet "worksheet.y" "${WORKSHEET_OUT}" DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/worksheet.h" COMPILE_FLAGS "--debug --graph")
#add_flex_bison_dependency(LEXER PARSER)

add_executable(worksheet
  "${WORKSHEET_OUT}"
)

target_include_directories(worksheet
  PUBLIC "${PROJECT_SOURCE_DIR}/third_party/libexpat/lib/"
  PUBLIC "${PROJECT_SOURCE_DIR}/third_party/minizip-ng/"
)
  #"."
  #"${PROJECT_SOURCE_DIR}/include"
  #"${PROJECT_SOURCE_DIR}/third_party/libcsv/"
  #"${CMAKE_BINARY_DIR}/configured_files/include"
  #"${CMAKE_BINARY_DIR}/third_party/minizip-ng/"

target_link_libraries(worksheet minizip)
target_link_libraries(worksheet expat)

#target_link_libraries(worksheet m)
