cmake_minimum_required(VERSION 3.5)
project(eve VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)

#add_subdirectory(eve-frontend)
#add_subdirectory(third-party/QCBOR)

add_custom_target (
  eve-frontend ALL
  COMMAND cd ${CMAKE_SOURCE_DIR}/eve-frontend && npm run build -- --outDir ${CMAKE_CURRENT_BINARY_DIR}/dist
)

#[[
add_library(anthidium SHARED
    anthidium/animation.cpp
    anthidium/buffers.cpp
)
]]

# eve
add_executable(eve
    eve-backend/eve.c
)

target_include_directories(eve PUBLIC 
    eve-backend
    /usr/local/include
)

target_link_directories(eve PUBLIC
    /usr/local/lib
)

target_link_libraries(eve
    websockets
    ev
    eio
    confuse
    sodium
    qcbor
)
    #qcbor -static

target_link_libraries(eve
)
# hal
add_executable(hal
    hal/hal.c
    hal/monocypher/monocypher.c
)

target_include_directories(hal PUBLIC 
    hal
    hal/libev-4.33
    hal/monocypher
)

target_link_directories(hal PUBLIC
    #/usr/local/lib
)

target_link_libraries(hal
    #sodium
)

#[[
target_include_directories(hal PUBLIC 
    include
    include/hashmap
    src
    /usr/include/google
    /usr/src/git/openwrt/libnl-tiny/include/
    src/libev-4.33
)
    #/usr/src/git/github/sabotage-linux/libnl-tiny/include/
]]

# ./config no-shared "-static -static-libgcc" no-idea no-mdc2 no-rc5 no-ssl2 no-ssl3 no-threads
# ./config no-shared "-static -static-libgcc" linux-armv4 --cross-compile-prefix=arm-linux-gnueabihf- no-idea no-mdc2 no-rc5 no-ssl2 no-ssl3
#set(OPENSSL_USE_STATIC_LIBS TRUE)
#find_package(OpenSSL REQUIRED)

#add_executable(${PROJECT_NAME} ${SOURCE_FILES})

#[[
#target_link_directories(hal PUBLIC lib)
target_link_libraries(hal protobuf-c)
target_link_libraries(hal /usr/src/git/hal/lib/armhf/libnl-tiny.a -static)
target_link_libraries(hal /usr/src/git/hal/lib/armhf/libhashmap.a -static)
#target_link_libraries(hal /usr/src/git/hal/lib/amd64/libnl-tiny.a -static)
#target_link_libraries(hal /usr/src/git/hal/lib/amd64/libhashmap.a -static)
target_link_libraries(hal -static)
set_target_properties(hal PROPERTIES LINK_FLAGS_RELEASE -s)

set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O3")

find_program(asciidoctor_exe NAMES asciidoctor)
find_program(antora_exe NAMES antora)
]]
