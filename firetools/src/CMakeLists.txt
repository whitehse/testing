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


add_library(firetools STATIC
  "${LIBRARY_SOURCES}"
)
  #"${LIBRARY_HEADERS"

add_executable(test 
  "${EXE_SOURCES}"
)

if (WASM)
else()
  target_link_libraries(test m)
endif()

#target_link_options(test PRIVATE "LINKER:--no-check-features")
