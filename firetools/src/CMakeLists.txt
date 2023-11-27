set(LIBRARY_SOURCES
  "amortization.c"
  "annuity.c"
  "interest.c"
  "loan_paydown.c"
)

#set(LIBRARY_HEADERS
#)

set(EXE_SOURCES
  "main.c"
)

include_directories(
  "."
  "${PROJECT_SOURCE_DIR}/include"
  "${CMAKE_BINARY_DIR}/configured_files/include")


add_library(${LIBRARY_NAME} STATIC
  "${LIBRARY_SOURCES}"
)
  #"${LIBRARY_HEADERS"

add_executable(exe
  "${EXE_SOURCES}"
)

#if (WASM)
#else()
  target_link_libraries(exe m)
#endif()

#target_link_options(main PRIVATE "LINKER:--no-check-features")

set(LEXER_OUT "${PARSER_DIR}/lexer.c")
set(PARSER_OUT "${PARSER_DIR}/parser.c")

add_subdirectory(parser)
