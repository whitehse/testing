%{
  #include <stdio.h>
  #include <math.h>
  #include <hashmap.h>
  int yylex (void);
  void yyerror (char const *);

  int xml_depth=0;
%}

%union {
  int intval;
  float floatval;
  char *charval;
}

%token NUM
%token <charval> TAGBEGIN
%token TAGEND
%token <charval> TAGCLOSE
%token TAGENDANDCLOSE
%token <charval> ATTRIBUTENAME
%token EQUALSIGN
%token <charval> ATTRIBUTEVALUE
%token <charval> CONTENT

%define parse.error verbose

%%

/*
exp:
  TAGBEGIN       { printf("%*c<%s", xml_depth*2, ' ', $1); xml_depth += 1; }
| TAGEND         { printf(">\n"); }
| TAGCLOSE       { printf("%*c</%s>\n", xml_depth*2-2, ' ', $1); xml_depth -= 1; }
| ATTRIBUTENAME  { printf(" ATTR:%s=", $1); }
| ATTRIBUTEVALUE { printf("%s", $1); }
| CONTENT        { printf("%*cCONTENT:%s\n", xml_depth*2, ' ', $1); }
| exp exp
;
*/

/*syntax error, unexpected ATTRIBUTENAME, expecting CONTENT*/

tags: tags tag;
tags: tag;

tag: TAGBEGIN attributes_opt CONTENT TAGEND;
tag: TAGBEGIN CONTENT TAGEND;

attributepair: ATTRIBUTENAME ATTRIBUTEVALUE;
attribute_opt: attributepair;
attributes_opt: attribute_opt attributepair;




/*attribute_opt: %empty;*/

/*attributes_opt: %empty;*/

/*
exp: TAGBEGIN       { printf("%*c<%s", xml_depth*2, ' ', $1); xml_depth += 1; }
exp: TAGEND         { printf(">\n"); }
exp: TAGCLOSE       { printf("%*c</%s>\n", xml_depth*2-2, ' ', $1); xml_depth -= 1; }
exp: ATTRIBUTENAME  { printf(" ATTR:%s=", $1); }
exp: ATTRIBUTEVALUE { printf("%s", $1); }
exp: CONTENT        { printf("%*cCONTENT:%s\n", xml_depth*2, ' ', $1); }
exp: exp exp
;
*/

%%
//#include <stdio.h>
#include <string.h>
#include <sys/queue.h>
#include <ctype.h>
#include <expat.h>
#include <mz.h>
#include "mz_strm.h"
#include <mz_strm_mem.h>
#include "mz_zip.h"

char file_buf[4096];
int file_bytes_read;

char *buf;

struct token_tailq {
    int token_id;
    union {
      int intval;
      float floatval;
      char *charval;
    };
    TAILQ_ENTRY(token_tailq) tokens;
};

TAILQ_HEAD(token_tailq_head, token_tailq);
struct token_tailq_head *token_head;

static void XMLCALL startElement(void *userData, const XML_Char *name, const XML_Char **atts) {
  int i;
  int *const depthPtr = (int *)userData;
  (void)atts;

  for (i = 0; i < *depthPtr; i++) {
    //putchar('\t');
    //printf("%s\n", name);
    *depthPtr += 1;
  }

  struct token_tailq *token = malloc(sizeof(*token));
  token->token_id = TAGBEGIN;
  token->charval = strdup(name);
  TAILQ_INSERT_TAIL(token_head, token, tokens);

  for (i = 0; atts[i]; i += 2) {
    //printf(" %" XML_FMT_STR "='%" XML_FMT_STR "'", atts[i], atts[i + 1]);
    token = malloc(sizeof(*token));
    token->token_id = ATTRIBUTENAME;
    token->charval = strdup(atts[i]);
    TAILQ_INSERT_TAIL(token_head, token, tokens);

    token = malloc(sizeof(*token));
    token->token_id = ATTRIBUTEVALUE;
    token->charval = strdup(atts[i+1]);
    TAILQ_INSERT_TAIL(token_head, token, tokens);
  }

  token = malloc(sizeof(*token));
  token->token_id = TAGEND;
  TAILQ_INSERT_TAIL(token_head, token, tokens);
/*
%token TAGBEGIN
%token TAGEND
%token TAGCLOSE
%token TAGENDANDCLOSE
%token ATTRIBUTENAME
%token EQUALSIGN
%token ATTRIBUTEVALUE
*/

}

static void XMLCALL endElement(void *userData, const XML_Char *name) {
  //int *const depthPtr = (int *)userData;
  //(void)name;
  //*depthPtr -= 1;

  struct token_tailq *token = malloc(sizeof(*token));
  token->token_id = TAGCLOSE;
  token->charval = strdup(name);
  TAILQ_INSERT_TAIL(token_head, token, tokens);
}

static void XMLCALL charHandler(void *userData, const XML_Char *s, int len) {
  struct token_tailq *token = malloc(sizeof(*token));
  token->token_id = CONTENT;
  token->charval = calloc(len+1, sizeof(char));
  strncpy(token->charval, s, len);
  TAILQ_INSERT_TAIL(token_head, token, tokens);
}

int doit(void *data, int length) {
  uint8_t *zip_buffer = NULL;
  int32_t zip_buffer_size = 0;
  void *stream = NULL;
  void *zip_handle = NULL;
  int ret;

  stream = mz_stream_mem_create();

  mz_stream_mem_set_buffer(stream, data, length);
  mz_stream_open(stream, NULL, MZ_OPEN_MODE_READ);

  zip_handle = mz_zip_create();
  ret = mz_zip_open(zip_handle, stream, MZ_OPEN_MODE_READ);

  char *char_data = (char *)data;
  uint64_t count;
  ret = mz_zip_get_number_entry(zip_handle, &count);

  mz_zip_file *file_info;
  //ret = mz_zip_locate_entry(zip_handle, "[Content_Types].xml", 0);
  //ret = mz_zip_locate_entry(zip_handle, "xl/worksheets/sheet1.xml", 0);
  ret = mz_zip_locate_entry(zip_handle, "xl/workbook.xml", 0);
  ret = mz_zip_entry_read_open(zip_handle, 0, NULL);
  if (ret != MZ_OK) {
  }

  XML_Parser parser = XML_ParserCreate(NULL);

  int done;
  int depth = 0;

  XML_SetUserData(parser, &depth);
  XML_SetElementHandler(parser, startElement, endElement);
  XML_SetCharacterDataHandler(parser, charHandler);
  while (file_bytes_read = mz_zip_entry_read(zip_handle, file_buf, sizeof(file_buf))) {
      XML_Parse(parser, file_buf, file_bytes_read, 0);
  }
  XML_ParserFree(parser);

  mz_zip_close(zip_handle);
  mz_zip_delete(&zip_handle);
  mz_stream_mem_delete(&stream);

  return length;
}

int yylex (void) {
  int ret = 0;
  struct token_tailq *token_item;
  token_item = TAILQ_FIRST(token_head);
  if (token_item != NULL) {
    yylval.charval = token_item->charval;
    ret = token_item->token_id;
		TAILQ_REMOVE(token_head, token_item, tokens);
		free(token_item);
  }
  return ret;
}

int main (void) {
  #ifdef YYDEBUG
    yydebug = 1;
  #endif

  token_head = malloc(sizeof(*token_head));
  TAILQ_INIT(token_head);

  struct token_tailq *token_item = malloc(sizeof(*token_item));

  FILE *f = fopen("/tmp/test.xlsx", "rb");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

  buf = malloc(fsize);
  fread(buf, fsize, 1, f);
  fclose(f);

  doit (buf, fsize);
  free(buf);

  return yyparse ();
}

void yyerror (char const *s) {
  fprintf (stderr, "%s\n", s);
}
