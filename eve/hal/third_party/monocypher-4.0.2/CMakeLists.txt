project (monocypher)

#add_library(monocypher SHARED
#    src/monocypher.c
#)

#add_custom_command(OUTPUT lib/libmonocypher.so.4
#                   COMMAND make
#                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#)

add_custom_target(monocypher
                   COMMAND make
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
