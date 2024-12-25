/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: gperf -D -s 10 -t spreadsheetml_types.gperf  */
/* Computed positions: -k'58,61,71' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif

#line 1 "spreadsheetml_types.gperf"

#include <assert.h>
#include <spreadsheetmlrw.h>
//#include <expat.h>
//#include <states.h>
int return_ml_type (struct ml_type_parse *ml_type);
#line 17 "spreadsheetml_types.gperf"
struct ml_type_parse;
#include <string.h>

#define ML_TYPE_TOTAL_KEYWORDS 76
#define ML_TYPE_MIN_WORD_LENGTH 15
#define ML_TYPE_MAX_WORD_LENGTH 88
#define ML_TYPE_MIN_HASH_VALUE 15
#define ML_TYPE_MAX_HASH_VALUE 163
/* maximum key range = 149, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash_ml_type (register const char *str, register size_t len)
{
  static unsigned char asso_values[] =
    {
      164, 164, 164, 164, 164, 164, 164, 164, 164, 164,
      164, 164, 164, 164, 164, 164, 164, 164, 164, 164,
      164, 164, 164, 164, 164, 164, 164, 164, 164, 164,
      164, 164, 164, 164, 164, 164, 164, 164, 164, 164,
      164, 164, 164,   0, 164, 164, 164, 164, 164, 164,
      164, 164, 164, 164, 164, 164, 164, 164, 164, 164,
      164, 164, 164, 164, 164, 164, 164,   0,  20, 164,
      164, 164, 164, 164, 164, 164, 164, 164, 164, 164,
      164, 164,   0, 164, 164, 164, 164, 164, 164, 164,
      164, 164, 164, 164, 164, 164, 164,   0,  75,   5,
        0,  10,  10,   0,  45,  30, 164, 164,  50,  30,
       15,   0,  15,  25,  25,  35,   0,   0,  25,  10,
       50,  70, 164, 164, 164, 164, 164, 164
    };
  register unsigned int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[70]];
      /*FALLTHROUGH*/
      case 70:
      case 69:
      case 68:
      case 67:
      case 66:
      case 65:
      case 64:
      case 63:
      case 62:
      case 61:
        hval += asso_values[(unsigned char)str[60]];
      /*FALLTHROUGH*/
      case 60:
      case 59:
      case 58:
        hval += asso_values[(unsigned char)str[57]];
      /*FALLTHROUGH*/
      case 57:
      case 56:
      case 55:
      case 54:
      case 53:
      case 52:
      case 51:
      case 50:
      case 49:
      case 48:
      case 47:
      case 46:
      case 45:
      case 44:
      case 43:
      case 42:
      case 41:
      case 40:
      case 39:
      case 38:
      case 37:
      case 36:
      case 35:
      case 34:
      case 33:
      case 32:
      case 31:
      case 30:
      case 29:
      case 28:
      case 27:
      case 26:
      case 25:
      case 24:
      case 23:
      case 22:
      case 21:
      case 20:
      case 19:
      case 18:
      case 17:
      case 16:
      case 15:
        break;
    }
  return hval;
}

struct ml_type_parse *
in_word_set_ml_type (register const char *str, register size_t len)
{
  static struct ml_type_parse wordlist_ml_type[] =
    {
#line 33 "spreadsheetml_types.gperf"
      {"application/xml",                                                                          ML_XML_MAPPING },
#line 39 "spreadsheetml_types.gperf"
      {"application/vnd.openxmlformats-officedocument.drawing+xml",                                ML_DRAWING},
#line 20 "spreadsheetml_types.gperf"
      {"http://purl.oclc.org/ooxml/officeDocument/relationships/calcChain",                        ML_CALC_CHAIN},
#line 31 "spreadsheetml_types.gperf"
      {"http://purl.oclc.org/ooxml/officeDocument/relationships/customProperty",                   ML_CUSTOM_PROPERTY},
#line 79 "spreadsheetml_types.gperf"
      {"http://purl.oclc.org/ooxml/officeDocument/relationships/table",                            ML_TABLE},
#line 76 "spreadsheetml_types.gperf"
      {"http://purl.oclc.org/ooxml/officeDocument/relationships/styles",                           ML_STYLE_SHEET},
#line 26 "spreadsheetml_types.gperf"
      {"http://purl.oclc.org/ooxml/officeDocument/relationships/comments",                         ML_COMMENTS},
#line 41 "spreadsheetml_types.gperf"
      {"http://schemas.openxmlformats.org/officeDocument/2006/relationships/drawing",              ML_DRAWING},
#line 29 "spreadsheetml_types.gperf"
      {"http://purl.oclc.org/ooxml/officeDocument/relationships/connections",                      ML_CONNECTIONS},
#line 24 "spreadsheetml_types.gperf"
      {"http://schemas.openxmlformats.org/officeDocument/2006/relationships/chartsheet",           ML_CHART_SHEET},
#line 38 "spreadsheetml_types.gperf"
      {"http://schemas.openxmlformats.org/officeDocument/2006/relationships/dialogsheet",          ML_DIALOG_SHEET},
#line 44 "spreadsheetml_types.gperf"
      {"http://schemas.openxmlformats.org/officeDocument/2006/relationships/externalLink",         ML_EXTERNAL_LINK},
#line 62 "spreadsheetml_types.gperf"
      {"http://schemas.openxmlformats.org/officeDocument/2006/relationships/sharedStrings",        ML_SHARED_STRING_TABLE},
#line 94 "spreadsheetml_types.gperf"
      {"http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme",                ML_THEME},
#line 92 "spreadsheetml_types.gperf"
      {"http://schemas.openxmlformats.org/officeDocument/2006/relationships/externalLinkPath",     ML_EXTERNAL_WORKBOOK},
#line 86 "spreadsheetml_types.gperf"
      {"http://purl.oclc.org/ooxml/officeDocument/relationships/officeDocument",                   ML_WORKBOOK},
#line 82 "spreadsheetml_types.gperf"
      {"http://purl.oclc.org/ooxml/officeDocument/relationships/volatileDependencies",             ML_VOLATILE_TYPES},
#line 71 "spreadsheetml_types.gperf"
      {"http://schemas.openxmlformats.org/officeDocument/2006/relationships/usernames",            ML_USERS},
#line 59 "spreadsheetml_types.gperf"
      {"http://schemas.openxmlformats.org/officeDocument/2006/relationships/queryTable",           ML_QUERY_TABLE},
#line 47 "spreadsheetml_types.gperf"
      {"http://schemas.openxmlformats.org/officeDocument/2006/relationships/sheetMetadata",        ML_METADATA},
#line 87 "spreadsheetml_types.gperf"
      {"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument",       ML_WORKBOOK},
#line 34 "spreadsheetml_types.gperf"
      {"http://purl.oclc.org/ooxml/officeDocument/relationships/xmlMaps",                          ML_XML_MAPPING},
#line 30 "spreadsheetml_types.gperf"
      {"http://schemas.openxmlformats.org/officeDocument/2006/relationships/connections",          ML_CONNECTIONS},
#line 49 "spreadsheetml_types.gperf"
      {"http://purl.oclc.org/ooxml/officeDocument/relationships/pivotTable",                       ML_PIVOT_TABLE_DEFINITION},
#line 37 "spreadsheetml_types.gperf"
      {"http://purl.oclc.org/ooxml/officeDocument/relationships/dialogsheet",                      ML_DIALOG_SHEET},
#line 78 "spreadsheetml_types.gperf"
      {"application/vnd.openxmlformats-officedocument.spreadsheetml.table+xml",                    ML_TABLE},
#line 89 "spreadsheetml_types.gperf"
      {"http://purl.oclc.org/ooxml/officeDocument/relationships/worksheet",                        ML_WORKSHEET},
#line 90 "spreadsheetml_types.gperf"
      {"http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet",            ML_WORKSHEET},
#line 50 "spreadsheetml_types.gperf"
      {"http://schemas.openxmlformats.org/officeDocument/2006/relationships/pivotTable",           ML_PIVOT_TABLE_DEFINITION},
#line 68 "spreadsheetml_types.gperf"
      {"http://schemas.openxmlformats.org/officeDocument/2006/relationships/revisionLog",          ML_REVISION_LOG},
#line 36 "spreadsheetml_types.gperf"
      {"application/vnd.openxmlformats-officedocument.spreadsheetml.dialogsheet+xml",              ML_DIALOG_SHEET},
#line 27 "spreadsheetml_types.gperf"
      {"http://schemas.openxmlformats.org/officeDocument/2006/relationships/comments",             ML_COMMENTS},
#line 85 "spreadsheetml_types.gperf"
      {"application/vnd.openxmlformats-officedocument.spreadsheetml.template.main+xml",            ML_WORKBOOK},
#line 65 "spreadsheetml_types.gperf"
      {"http://schemas.openxmlformats.org/officeDocument/2006/relationships/revisionHeaders",      ML_REVISION_HEADERS},
#line 22 "spreadsheetml_types.gperf"
      {"application/vnd.openxmlformats-officedocument.spreadsheetml.chartsheet+xml",               ML_CHART_SHEET},
#line 56 "spreadsheetml_types.gperf"
      {"http://schemas.openxmlformats.org/officeDocument/2006/relationships/pivotCacheRecords",    ML_PIVOT_CACHE_RECORDS},
#line 23 "spreadsheetml_types.gperf"
      {"http://purl.oclc.org/ooxml/officeDocument/relationships/chartsheet",                       ML_CHART_SHEET},
#line 67 "spreadsheetml_types.gperf"
      {"http://purl.oclc.org/ooxml/officeDocument/relationships/revisionLog",                      ML_REVISION_LOG},
#line 53 "spreadsheetml_types.gperf"
      {"http://schemas.openxmlformats.org/officeDocument/2006/relationships/pivotCacheDefinition", ML_PIVOT_CACHE_DEFINITION },
#line 46 "spreadsheetml_types.gperf"
      {"http://purl.oclc.org/ooxml/officeDocument/relationships/sheetMetadata",                    ML_METADATA},
#line 70 "spreadsheetml_types.gperf"
      {"http://purl.oclc.org/ooxml/officeDocument/relationships/usernames",                        ML_USERS},
#line 93 "spreadsheetml_types.gperf"
      {"http://purl.oclc.org/ooxml/officeDocument/relationships/theme",                            ML_THEME},
#line 32 "spreadsheetml_types.gperf"
      {"http://schemas.openxmlformats.org/officeDocument/2006/relationships/customProperty",       ML_CUSTOM_PROPERTY},
#line 40 "spreadsheetml_types.gperf"
      {"http://purl.oclc.org/ooxml/officeDocument/relationships/drawing",                          ML_DRAWING},
#line 48 "spreadsheetml_types.gperf"
      {"application/vnd.openxmlformats-officedocument.spreadsheetml.pivotTable+xml",               ML_PIVOT_TABLE_DEFINITION},
#line 72 "spreadsheetml_types.gperf"
      {"application/vnd.openxmlformats-officedocument.spreadsheetml.tableSingleCells+xml",         ML_SINGLE_CELLS},
#line 52 "spreadsheetml_types.gperf"
      {"http://purl.oclc.org/ooxml/officeDocument/relationships/pivotCacheDefinition",             ML_PIVOT_CACHE_DEFINITION},
#line 61 "spreadsheetml_types.gperf"
      {"http://purl.oclc.org/ooxml/officeDocument/relationships/sharedStrings",                    ML_SHARED_STRING_TABLE},
#line 35 "spreadsheetml_types.gperf"
      {"http://schemas.openxmlformats.org/officeDocument/2006/relationships/xmlMaps",              ML_XML_MAPPING},
#line 54 "spreadsheetml_types.gperf"
      {"application/vnd.openxmlformats-officedocument.spreadsheetml.pivotCacheRecords+xml",        ML_PIVOT_CACHE_RECORDS},
#line 21 "spreadsheetml_types.gperf"
      {"http://schemas.openxmlformats.org/officeDocument/2006/relationships/calcChain",            ML_CALC_CHAIN},
#line 55 "spreadsheetml_types.gperf"
      {"http://purl.oclc.org/ooxml/officeDocument/relationships/pivotCacheRecords",                ML_PIVOT_CACHE_RECORDS},
#line 57 "spreadsheetml_types.gperf"
      {"application/vnd.openxmlformats-officedocument.spreadsheetml.queryTable+xml",               ML_QUERY_TABLE},
#line 66 "spreadsheetml_types.gperf"
      {"application/vnd.openxmlformats-officedocument.spreadsheetml.revisionLog+xml",              ML_REVISION_LOG},
#line 42 "spreadsheetml_types.gperf"
      {"application/vnd.openxmlformats-officedocument.spreadsheetml.externalLink+xml",             ML_EXTERNAL_LINK},
#line 73 "spreadsheetml_types.gperf"
      {"http://purl.oclc.org/ooxml/officeDocument/relationships/tableSingleCells",                 ML_SINGLE_CELLS},
#line 63 "spreadsheetml_types.gperf"
      {"application/vnd.openxmlformats-officedocument.spreadsheetml.revisionHeaders+xml",          ML_REVISION_HEADERS},
#line 75 "spreadsheetml_types.gperf"
      {"application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml",                   ML_STYLE_SHEET},
#line 58 "spreadsheetml_types.gperf"
      {"http://purl.oclc.org/ooxml/officeDocument/relationships/queryTable",                       ML_QUERY_TABLE},
#line 25 "spreadsheetml_types.gperf"
      {"application/vnd.openxmlformats-officedocument.spreadsheetml.comments+xml",                 ML_COMMENTS},
#line 83 "spreadsheetml_types.gperf"
      {"http://schemas.openxmlformats.org/officeDocument/2006/relationships/volatileDependencies", ML_VOLATILE_TYPES},
#line 84 "spreadsheetml_types.gperf"
      {"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml",               ML_WORKBOOK},
#line 45 "spreadsheetml_types.gperf"
      {"application/vnd.openxmlformats-officedocument.spreadsheetml.sheetMetadata+xml",            ML_METADATA},
#line 43 "spreadsheetml_types.gperf"
      {"http://purl.oclc.org/ooxml/officeDocument/relationships/externalLink",                     ML_EXTERNAL_LINK},
#line 77 "spreadsheetml_types.gperf"
      {"http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles",               ML_STYLE_SHEET},
#line 28 "spreadsheetml_types.gperf"
      {"application/vnd.openxmlformats-officedocument.spreadsheetml.connections+xml",              ML_CONNECTIONS},
#line 91 "spreadsheetml_types.gperf"
      {"http://purl.oclc.org/ooxml/officeDocument/relationships/externalLinkPath",                 ML_EXTERNAL_WORKBOOK},
#line 80 "spreadsheetml_types.gperf"
      {"http://schemas.openxmlformats.org/officeDocument/2006/relationships/table",                ML_TABLE},
#line 51 "spreadsheetml_types.gperf"
      {"application/vnd.openxmlformats-officedocument.spreadsheetml.pivotCacheDefinition+xml",     ML_PIVOT_CACHE_DEFINITION},
#line 64 "spreadsheetml_types.gperf"
      {"http://purl.oclc.org/ooxml/officeDocument/relationships/revisionHeaders",                  ML_REVISION_HEADERS},
#line 69 "spreadsheetml_types.gperf"
      {"application/vnd.openxmlformats-officedocument.spreadsheetml.userNames+xml",                ML_USERS},
#line 81 "spreadsheetml_types.gperf"
      {"application/vnd.openxmlformats-officedocument.spreadsheetml.volatileDependencies+xml",     ML_VOLATILE_TYPES},
#line 60 "spreadsheetml_types.gperf"
      {"application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml",            ML_SHARED_STRING_TABLE},
#line 19 "spreadsheetml_types.gperf"
      {"application/vnd.openxmlformats-officedocument.spreadsheetml.calcChain+xml",                ML_CALC_CHAIN},
#line 74 "spreadsheetml_types.gperf"
      {"http://schemas.openxmlformats.org/officeDocument/2006/relationships/tableSingleCells",     ML_SINGLE_CELLS},
#line 88 "spreadsheetml_types.gperf"
      {"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml",                ML_WORKSHEET}
    };

  static signed char lookup[] =
    {
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1,  1, -1, -1, -1, -1, -1, -1, -1,  2, -1, -1, -1, -1,
       3,  4,  5, -1,  6,  7, -1,  8,  9, 10, 11, 12, -1, 13,
      14, 15, 16, 17, 18, -1, -1, 19, 20, 21, 22, -1, 23, 24,
      -1, 25, 26, -1, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
      37, 38, 39, 40, 41, 42, 43, 44, 45, 46, -1, -1, 47, 48,
      49, 50, 51, 52, 53, 54, 55, -1, 56, 57, 58, 59, 60, 61,
      -1, -1, 62, 63, 64, 65, -1, 66, 67, 68, -1, 69, -1, 70,
      71, -1, -1, 72, 73, 74, -1, -1, -1, 75
    };

  if (len <= ML_TYPE_MAX_WORD_LENGTH && len >= ML_TYPE_MIN_WORD_LENGTH)
    {
      register unsigned int key = hash_ml_type (str, len);

      if (key <= ML_TYPE_MAX_HASH_VALUE)
        {
          register int index = lookup[key];

          if (index >= 0)
            {
              register const char *s = wordlist_ml_type[index].name;

              if (*str == *s && !strcmp (str + 1, s + 1))
                return &wordlist_ml_type[index];
            }
        }
    }
  return 0;
}
#line 95 "spreadsheetml_types.gperf"


/*
struct ml_type_parse {
  char* name;
  ML_Type ml_type;
};
*/

//typedef enum {POINTER, STRING, INT, FLOAT} DataType;

/*
typedef struct node
{
    DataType t;
    union {
        void *pointer;
        char *string;
        int integer;
        float floating;
    } data;
    struct node *left,
                *right;
} Node;
*/

/*
struct mlrw_string {
  char* key;
  int   len;
}

struct SpreadSheet {
  mlrw_string *calc_chain;
};
*/

static unsigned int
hash_ml_type (register const char *str, register size_t len);


struct ml_type_parse *
in_word_set_ml_type (register const char *str, register size_t len);

/*
%define hash-function-name hash_ml_type
%define lookup-function-name in_word_set_ml_type
%define constants-prefix ML_TYPE_
%define word-array-name wordlist_ml_type
*/
