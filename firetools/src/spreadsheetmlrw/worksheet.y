/* Reverse polish notation calculator.  */


%{
  #include <stdio.h>
  #include <math.h>
  int yylex (void);
  void yyerror (char const *);
%}


%define api.value.type {double}
%token NUM

%% /* Grammar rules and actions follow.  */

input:
  %empty
| input line
;



line:
  '\n'
| exp '\n'      { printf ("%.10g\n", $1); }
;



exp:
  NUM           { $$ = $1;           }
| exp exp '+'   { $$ = $1 + $2;      }
| exp exp '-'   { $$ = $1 - $2;      }
| exp exp '*'   { $$ = $1 * $2;      }
| exp exp '/'   { $$ = $1 / $2;      }
| exp exp '^'   { $$ = pow ($1, $2); }  /* Exponentiation */
| exp 'n'       { $$ = -$1;          }  /* Unary minus    */
;

%%

/* The lexical analyzer returns a double floating point
   number on the stack and the token NUM, or the numeric code
   of the character read if not a number.  It skips all blanks
   and tabs, and returns 0 for end-of-input.  */

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

static void XMLCALL startElement(void *userData, const XML_Char *name, const XML_Char **atts) {
  int i;
  int *const depthPtr = (int *)userData;
  (void)atts;

  for (i = 0; i < *depthPtr; i++)
    putchar('\t');
    //;
  //printf("%" XML_FMT_STR "\n", name);
  printf("%s\n", name);
  //string_len = strlen(name);
  //memcpy (string_buffer, name, string_len);
  //imported_func(*depthPtr);
  *depthPtr += 1;
}

static void XMLCALL endElement(void *userData, const XML_Char *name) {
  int *const depthPtr = (int *)userData;
  (void)name;

  *depthPtr -= 1;
}

TAILQ_HEAD(tailhead, entry) head;

struct entry {
  char c;
  TAILQ_ENTRY(entry) entries;
};

void add_to_queue(char ch) {
  struct entry *elem;
  elem = malloc(sizeof(struct entry));
  if (elem) {
    elem->c = ch;
  }
  TAILQ_INSERT_HEAD(&head, elem, entries);
}

int doit(void *data, int length) {
  uint8_t *zip_buffer = NULL;
  int32_t zip_buffer_size = 0;
  void *stream = NULL;
  void *zip_handle = NULL;
  int ret;

  //memcpy (string_buffer, "stuff", 5);
  //string_len = 5;

  stream = mz_stream_mem_create();

  mz_stream_mem_set_buffer(stream, data, length);
  mz_stream_open(stream, NULL, MZ_OPEN_MODE_READ);

  zip_handle = mz_zip_create();
  ret = mz_zip_open(zip_handle, stream, MZ_OPEN_MODE_READ);
  //imported_func(ret);

  char *char_data = (char *)data;
  //imported_func((int)space[0]);
  //imported_func((int)space[1]);

  /* TODO: unzip operations.. */
  uint64_t count;
  ret = mz_zip_get_number_entry(zip_handle, &count);
  //imported_func((int)count);

  mz_zip_file *file_info;
  //imported_func(ret);
  ret = mz_zip_locate_entry(zip_handle, "[Content_Types].xml", 0);
  ret = mz_zip_entry_read_open(zip_handle, 0, NULL);
  if (ret != MZ_OK) {
    //imported_func(ret);
    //memcpy (string_buffer, "Unable to open file inside zip", 30);
    //string_len = 30;
    //imported_func(1);
  }
  //memcpy (string_buffer, "Successfully read from file", 27);
  //string_len = 27;
  //imported_func(1);
  //string_len = mz_zip_entry_read(zip_handle, string_buffer, 16);
  //imported_func(1);
  file_bytes_read = mz_zip_entry_read(zip_handle, file_buf, sizeof(file_buf));
  //imported_func(file_bytes_read);

  XML_Parser parser = XML_ParserCreate(NULL);

  int done;
  int depth = 0;

  XML_SetUserData(parser, &depth);
  XML_SetElementHandler(parser, startElement, endElement);

  XML_Parse(parser, file_buf, file_bytes_read, 0);

  XML_ParserFree(parser);

  mz_zip_close(zip_handle);
  mz_zip_delete(&zip_handle);

  mz_stream_mem_delete(&stream);

  int *first_val = data;
  return length;
}

int yylex (void) {
  int c;

  /* Skip white space.  */
  while ((c = getchar ()) == ' ' || c == '\t')
    continue;


  /* Process numbers.  */
  if (c == '.' || isdigit (c)) {
      ungetc (c, stdin);
      scanf ("%lf", &yylval);
      return NUM;
  }

  /* Return end-of-input.  */
  if (c == EOF)
    return 0;
  /* Return a single char.  */
  return c;
}

int main (void) {
  FILE *f = fopen("/tmp/test.xlsx", "rb");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

  //string = malloc(fsize + 1);
  buf = malloc(fsize);
  //fread(string, fsize, 1, f);
  fread(buf, fsize, 1, f);
  fclose(f);

  doit (buf, fsize);

  return yyparse ();
}

//#include <stdio.h>

/* Called by yyparse on error.  */
void yyerror (char const *s) {
  //fprintf (stderr, "%s\n", s);
}

/*
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <spreadsheetmlrw.h>

//#include "mz_zip_rw.h"

#include <csv.h>
*/

/*
char string_buffer[256];
int string_len = 0;

//char file_buf[4096];
//int file_bytes_read;

void* get_string_buffer() {
  return &string_buffer;
}

int get_string_len() {
  return string_len;
}

extern void imported_func(int num);
*/

/*
int main() {
*/
/*
  char *source = NULL;
  long bufsize;
  FILE *fp = fopen("/tmp/test.xlsx", "r");
  if (fp != NULL) {
    if (fseek(fp, 0L, SEEK_END) == 0) {
        bufsize = ftell(fp);
        if (bufsize == -1) {

        source = malloc(sizeof(char) * (bufsize + 1));

        if (fseek(fp, 0L, SEEK_SET) != 0) {

        size_t newLen = fread(source, sizeof(char), bufsize, fp);
        if ( ferror( fp ) != 0 ) {
            fputs("Error reading file", stderr);
        } else {
            source[newLen++] = '\0';
        }
    }
    fclose(fp);
  }
*/
  //doit(source, bufsize);
/*
}
*/

/*
int test() {
  XML_Parser parser = XML_ParserCreate(NULL);
  struct csv_parser p;
  char buf[1024];
  size_t bytes_read;
  unsigned char options = 0;
  csv_init(&p, options);

  struct ml_type_parse *ml_type_parser;
  ml_type_parser = in_word_set_ml_type ("application/vnd.openxmlformats-officedocument.spreadsheetml.calcChain+xml", 73);

  //printf("Name = %s\n", ml_type_parser->name);
  //printf("Enum = %d\n", (int)ml_type_parser->ml_type);

  //ml_type_parser = in_word_set_ml_type ("application/vnd.openxmlformats-officedocument.spreadsheetml.chartsheet+xml", 74);

  //printf("Name = %s\n", ml_type_parser->name);
  //printf("Enum = %d\n", (int)ml_type_parser->ml_type);

  uint8_t *zip_buffer = NULL;
  int32_t zip_buffer_size = 0;
  void *stream = NULL;
  void *zip_handle = NULL;
  int ret;

*/
  /* TODO: fill zip_buffer with zip contents.. */
/*
  stream = mz_stream_mem_create();

  mz_stream_mem_set_buffer(stream, zip_buffer, zip_buffer_size);
  mz_stream_open(stream, NULL, MZ_OPEN_MODE_READ);

  zip_handle = mz_zip_create();
  ret = mz_zip_open(zip_handle, stream, MZ_OPEN_MODE_READ);
*/
  /* TODO: unzip operations.. */
/*
  ret = mz_zip_goto_first_entry(zip_handle);

  mz_zip_close(zip_handle);
  mz_zip_delete(&zip_handle);

  mz_stream_mem_delete(&stream);

  return 0;
}
*/