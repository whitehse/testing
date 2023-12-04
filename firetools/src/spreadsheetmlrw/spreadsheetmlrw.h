#include <stdlib.h>

typedef enum {
  // application/vnd.openxmlformats-officedocument.spreadsheetml.calcChain+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/calcChain
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/calcChain
  ML_CALC_CHAIN,
  // application/vnd.openxmlformats-officedocument.spreadsheetml.chartsheet+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/chartsheet
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/chartsheet
  ML_CHART_SHEET,
  // application/vnd.openxmlformats-officedocument.spreadsheetml.comments+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/comments
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/comments
  ML_COMMENTS,
  // application/vnd.openxmlformats-officedocument.spreadsheetml.connections+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/connections
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/connections
  ML_CONNECTIONS,
  // ?
  // http://purl.oclc.org/ooxml/officeDocument/relationships/customProperty
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/customProperty ?
  ML_CUSTOM_PROPERTY,
  // application/xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/xmlMaps
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/xmlMaps
  ML_XML_MAPPING, 
  // application/vnd.openxmlformats-officedocument.spreadsheetml.dialogsheet+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/dialogsheet
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/dialogsheet
  ML_DIALOG_SHEET,
  // application/vnd.openxmlformats-officedocument.drawing+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/drawing
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/drawing
  ML_DRAWING,
  // application/vnd.openxmlformats-officedocument.spreadsheetml.externalLink+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/externalLink
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/externalLink
  ML_EXTERNAL_LINK,
  ML_MAP_INFO,
  // application/vnd.openxmlformats-officedocument.spreadsheetml.sheetMetadata+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/sheetMetadata
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/sheetMetadata
  ML_METADATA,
  // application/vnd.openxmlformats-officedocument.spreadsheetml.pivotTable+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/pivotTable
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/pivotTable ?
  ML_PIVOT_TABLE_DEFINITION,
  // application/vnd.openxmlformats-officedocument.spreadsheetml.pivotCacheDefinition+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/pivotCacheDefinition
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/pivotCacheDefinition ?
  ML_PIVOT_CACHE_DEFINITION,
  // application/vnd.openxmlformats-officedocument.spreadsheetml.pivotCacheRecords+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/pivotCacheRecords
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/pivotCacheRecords ?
  ML_PIVOT_CACHE_RECORDS,
  // application/vnd.openxmlformats-officedocument.spreadsheetml.queryTable+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/queryTable
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/queryTable
  ML_QUERY_TABLE,
  // application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/sharedStrings
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/sharedStrings
  ML_SHARED_STRING_TABLE,
  // application/vnd.openxmlformats-officedocument.spreadsheetml.revisionHeaders+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/revisionHeaders
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/revisionHeaders ?
  ML_REVISION_HEADERS,
  // application/vnd.openxmlformats-officedocument.spreadsheetml.revisionLog+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/revisionLog
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/revisionLog ?
  ML_REVISION_LOG,
  // application/vnd.openxmlformats-officedocument.spreadsheetml.userNames+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/usernames
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/usernames ?
  ML_USERS,
  // application/vnd.openxmlformats-officedocument.spreadsheetml.tableSingleCells+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/tableSingleCells
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/tableSingleCells
  ML_SINGLE_CELLS,
  // application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/styles
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles
  ML_STYLE_SHEET,
  // application/vnd.openxmlformats-officedocument.spreadsheetml.table+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/table
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/table
  ML_TABLE,
  // application/vnd.openxmlformats-officedocument.spreadsheetml.volatileDependencies+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/volatileDependencies
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/volatileDependencies
  ML_VOLATILE_TYPES,
  // application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml
  // application/vnd.openxmlformats-officedocument.spreadsheetml.template.main+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/officeDocument
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument
  ML_WORKBOOK,
  // application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml
  // http://purl.oclc.org/ooxml/officeDocument/relationships/worksheet
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet
  ML_WORKSHEET,
  // ?
  // http://purl.oclc.org/ooxml/officeDocument/relationships/externalLinkPath
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/externalLinkPath
  ML_EXTERNAL_WORKBOOK,
  // ?
  // http://purl.oclc.org/ooxml/officeDocument/relationships/theme ?
  // http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme
  ML_THEME
} ML_Type;

struct ml_type_parse {
  char* name;
  ML_Type ml_type;
};

typedef enum {POINTER, STRING, INT, FLOAT} DataType;

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
