set(EXE_SOURCES
  hal.c
  ethernet.c
  ipv4.c
  udp.c
  tcp.c
  hashmap.c
)
  #axos.pb-c.c
  #objects.pb-c.c)
  #monocypher.c

#set(EXE_HEADERS
#)

add_executable(hal
  "${EXE_SOURCES}"
)
  #"${EXE_HEADERS}"

target_include_directories(hal PUBLIC 
    "${PROJECT_SOURCE_DIR}/include/"
    "${PROJECT_SOURCE_DIR}/third_party/include/"
    "${PROJECT_SOURCE_DIR}/third_party/include/hashmap"
    "${PROJECT_SOURCE_DIR}/third_party/include/libnl-tiny/"
    "${PROJECT_SOURCE_DIR}/third_party/libev/"
    "${PROJECT_SOURCE_DIR}/third_party/cJSON/"
    "${CMAKE_BINARY_DIR}/configured_files/include/"
    ".")
    #"${PROJECT_SOURCE_DIR}/third_party/monocypher/src/"

    #/usr/src/git/github/sabotage-linux/libnl-tiny/include/
    #/usr/include/google

if (ARM)
  target_link_libraries(hal "${PROJECT_SOURCE_DIR}/lib/armhf/libnl-tiny.a" -static)
  target_link_libraries(hal "${PROJECT_SOURCE_DIR}/lib/armhf/libhashmap.a" -static)
else()
  target_link_libraries(hal "${PROJECT_SOURCE_DIR}/lib/amd64/libnl-tiny.a" -static)
  target_link_libraries(hal "${PROJECT_SOURCE_DIR}/lib/amd64/libhashmap.a" -static)
endif()

#target_link_libraries(hal sodium -static)
target_link_libraries(hal cbor -static)
target_link_libraries(hal ${CMAKE_BINARY_DIR}/third_party/cJSON/libcjson.a -static)
target_link_libraries(hal -static)
set_target_properties(hal PROPERTIES LINK_FLAGS_RELEASE -s)
