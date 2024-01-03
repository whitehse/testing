set(LIBRARY_NAME "mygpu")

set(LIBRARY_SOURCES
  "mygpu.c"
)

set(LIBRARY_HEADERS
  "mygpu.h"
)

include_directories(
  "."
  "${PROJECT_SOURCE_DIR}/include"
  "${PROJECT_SOURCE_DIR}/third_party/webgpu/"
)

add_library(${LIBRARY_NAME} STATIC
  "${LIBRARY_SOURCES}"
)
  #"${LIBRARY_HEADERS"

#if (WASM)
#else()
#endif()
