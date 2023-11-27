# FLEX_TARGET(Name FlexInput FlexOutput [COMPILE_FLAGS <string>] [DEFINES_FILE <string>])
find_package(FLEX 2.6 REQUIRED)
# BISON_TARGET(<Name> <YaccInput> <CodeOutput> [COMPILE_FLAGS <flags>] [DEFINES_FILE <file>] [VERBOSE [<file>]] [REPORT_FILE <file>])
find_package(BISON 3.0 REQUIRED)

set(LEXER_OUT "lexer.c")
set(PARSER_OUT "parser.c")

flex_target(LEXER "lexer.l" "${LEXER_OUT}" DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/lexer.h")
bison_target(PARSER "parser.y" "${PARSER_OUT}" DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/parser.h")
add_flex_bison_dependency(LEXER PARSER)

add_executable(calc "${LEXER_OUT}" "${PARSER_OUT}")
