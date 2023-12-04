/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: gperf -D -s 10 -t spreadsheetml_types.gperf  */
/* Computed positions: -k'' */

#line 1 "spreadsheetml_types.gperf"

#include <assert.h>
#include <spreadsheetmlrw.h>
//#include <expat.h>
//#include <states.h>
int return_ml_type (struct ml_type_parse *ml_type);
#line 17 "spreadsheetml_types.gperf"
struct ml_type_parse;
#include <string.h>

#define ML_TYPE_TOTAL_KEYWORDS 6
#define ML_TYPE_MIN_WORD_LENGTH 65
#define ML_TYPE_MAX_WORD_LENGTH 78
#define ML_TYPE_MIN_HASH_VALUE 65
#define ML_TYPE_MAX_HASH_VALUE 78
/* maximum key range = 14, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
/*ARGSUSED*/
static unsigned int
hash_ml_type (register const char *str, register size_t len)
{
  return len;
}

struct ml_type_parse *
in_word_set_ml_type (register const char *str, register size_t len)
{
  static struct ml_type_parse wordlist_ml_type[] =
    {
#line 20 "spreadsheetml_types.gperf"
      {"http://purl.oclc.org/ooxml/officeDocument/relationships/calcChain",               ML_CALC_CHAIN},
#line 23 "spreadsheetml_types.gperf"
      {"http://purl.oclc.org/ooxml/officeDocument/relationships/chartsheet",              ML_CHART_SHEET},
#line 19 "spreadsheetml_types.gperf"
      {"application/vnd.openxmlformats-officedocument.spreadsheetml.calcChain+xml",       ML_CALC_CHAIN},
#line 22 "spreadsheetml_types.gperf"
      {"application/vnd.openxmlformats-officedocument.spreadsheetml.chartsheet+xml",      ML_CHART_SHEET},
#line 21 "spreadsheetml_types.gperf"
      {"http://schemas.openxmlformats.org/officeDocument/2006/relationships/calcChain",   ML_CALC_CHAIN},
#line 24 "spreadsheetml_types.gperf"
      {"http://schemas.openxmlformats.org/officeDocument/2006/relationships/chartsheet",  ML_CHART_SHEET}
    };

  static signed char lookup[] =
    {
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  1, -1, -1, -1,
      -1, -1, -1,  2,  3, -1, -1,  4,  5
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
#line 25 "spreadsheetml_types.gperf"

  // application/vnd.openxmlformats-officedocument.spreadsheetml.comments+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/comments
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/comments
  // application/vnd.openxmlformats-officedocument.spreadsheetml.connections+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/connections
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/connections
  // ?
  // http://purl.oclc.org/ooxml/officeDocument/relationships/customProperty
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/customProperty ?
  // application/xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/xmlMaps
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/xmlMaps
  // application/vnd.openxmlformats-officedocument.spreadsheetml.dialogsheet+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/dialogsheet
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/dialogsheet
  // application/vnd.openxmlformats-officedocument.drawing+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/drawing
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/drawing
  // application/vnd.openxmlformats-officedocument.spreadsheetml.externalLink+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/externalLink
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/externalLink
  // application/vnd.openxmlformats-officedocument.spreadsheetml.sheetMetadata+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/sheetMetadata
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/sheetMetadata
  // application/vnd.openxmlformats-officedocument.spreadsheetml.pivotTable+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/pivotTable
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/pivotTable ?
  // application/vnd.openxmlformats-officedocument.spreadsheetml.pivotCacheDefinition+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/pivotCacheDefinition
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/pivotCacheDefinition ?
  // application/vnd.openxmlformats-officedocument.spreadsheetml.pivotCacheRecords+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/pivotCacheRecords
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/pivotCacheRecords ?
  // application/vnd.openxmlformats-officedocument.spreadsheetml.queryTable+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/queryTable
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/queryTable
  // application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/sharedStrings
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/sharedStrings
  // application/vnd.openxmlformats-officedocument.spreadsheetml.revisionHeaders+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/revisionHeaders
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/revisionHeaders ?
  // application/vnd.openxmlformats-officedocument.spreadsheetml.revisionLog+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/revisionLog
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/revisionLog ?
  // application/vnd.openxmlformats-officedocument.spreadsheetml.userNames+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/usernames
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/usernames ?
  // application/vnd.openxmlformats-officedocument.spreadsheetml.tableSingleCells+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/tableSingleCells
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/tableSingleCells
  // application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/styles
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles
  // application/vnd.openxmlformats-officedocument.spreadsheetml.table+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/table
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/table
  // application/vnd.openxmlformats-officedocument.spreadsheetml.volatileDependencies+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/volatileDependencies
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/volatileDependencies
  // application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml
  // application/vnd.openxmlformats-officedocument.spreadsheetml.template.main+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/officeDocument
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument
  // application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/worksheet
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet
  // ?
  // http://purl.oclc.org/ooxml/officeDocument/relationships/externalLinkPath
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/externalLinkPath
  // ?
  // http://purl.oclc.org/ooxml/officeDocument/relationships/theme ?
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme
