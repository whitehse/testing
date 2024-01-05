#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <spreadsheetmlrw.h>

#include <mz.h>
#include "mz_strm.h"
#include <mz_strm_mem.h>
#include "mz_zip.h"

#include <expat.h>
#include <csv.h>

#include <stdio.h>
#include <stdlib.h>

#define BUF_SIZE 8192
#define MAX_NAMELEN 256

extern void imported_func(int num);

int main() {
  char *source = NULL;
  long bufsize;
  FILE *fp = fopen("/tmp/test.xlsx", "r");
  if (fp != NULL) {
    /* Go to the end of the file. */
    if (fseek(fp, 0L, SEEK_END) == 0) {
        /* Get the size of the file. */
        bufsize = ftell(fp);
        if (bufsize == -1) { /* Error */ }

        /* Allocate our buffer to that size. */
        source = malloc(sizeof(char) * (bufsize + 1));

        /* Go back to the start of the file. */
        if (fseek(fp, 0L, SEEK_SET) != 0) { /* Error */ }

        /* Read the entire file into memory. */
        size_t newLen = fread(source, sizeof(char), bufsize, fp);
        if ( ferror( fp ) != 0 ) {
            fputs("Error reading file", stderr);
        } else {
            source[newLen++] = '\0'; /* Just to be safe. */
        }
    }
    fclose(fp);
  }
  doit(source, bufsize);
}

int doit(void *data, int length) {
  uint8_t *zip_buffer = NULL;
  int32_t zip_buffer_size = 0;
  void *stream = NULL;
  void *zip_handle = NULL;
  int ret;

  //imported_func(42);
  stream = mz_stream_mem_create();

  mz_stream_mem_set_buffer(stream, data, length);
  mz_stream_open(stream, NULL, MZ_OPEN_MODE_READ);

  zip_handle = mz_zip_create();
  ret = mz_zip_open(zip_handle, stream, MZ_OPEN_MODE_READ);

  /* TODO: unzip operations.. */
  ret = mz_zip_goto_first_entry(zip_handle);
  //imported_func(ret);
  //imported_func(43);

  mz_zip_close(zip_handle);
  mz_zip_delete(&zip_handle);

  mz_stream_mem_delete(&stream);

  int *first_val = data;
  return length;
}

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

  /* TODO: fill zip_buffer with zip contents.. */

  stream = mz_stream_mem_create();

  mz_stream_mem_set_buffer(stream, zip_buffer, zip_buffer_size);
  mz_stream_open(stream, NULL, MZ_OPEN_MODE_READ);

  zip_handle = mz_zip_create();
  ret = mz_zip_open(zip_handle, stream, MZ_OPEN_MODE_READ);

  /* TODO: unzip operations.. */
  ret = mz_zip_goto_first_entry(zip_handle);

  mz_zip_close(zip_handle);
  mz_zip_delete(&zip_handle);

  mz_stream_mem_delete(&stream);

  //unzFile uf = unzOpen64("/tmp/file.xlsx");
  /*
  unz_global_info64 pglobal_info;
  int ret = unzGetGlobalInfo64(uf, &pglobal_info);
  if (ret != UNZ_OK) goto out;
  printf("The zip file contains %lu entries. The global comment is %d bytes long.\n", pglobal_info.number_entry, pglobal_info.size_comment);
  ret = unzGoToFirstFile(uf);
  if (ret != UNZ_OK) goto out;
  unz_file_info64 pfile_info;
  char filename[MAX_NAMELEN];
  ret = unzGetCurrentFileInfo64(uf, &pfile_info, filename, MAX_NAMELEN, NULL, 0, NULL, 0);
  if (ret != UNZ_OK) goto out;
  printf("The filename inside the zipfile is: \"%s\"\n", filename);

  char buf[BUF_SIZE];
  ret = unzOpenCurrentFile(uf);
  printf("Return value from unzOpenCurrentFile is %d\n", ret);
  if (ret != UNZ_OK) goto out;
  puts("File opened");

  ret = unzReadCurrentFile(uf, buf, BUF_SIZE);
  if (ret < 0) goto out;
  printf("file has %.*s\n", ret, buf);

  ret = unzCloseCurrentFile(uf);
  if (ret != UNZ_OK) goto out;

  ret = unzGoToNextFile(uf);

  while(ret == UNZ_OK) {
    ret = unzGetCurrentFileInfo64(uf, &pfile_info, filename, MAX_NAMELEN, NULL, 0, NULL, 0);
    if (ret != UNZ_OK) goto out;
    printf("The filename inside the zipfile is: \"%s\"\n", filename);
    ret = unzGoToNextFile(uf);
  }


out:
  ret = unzClose(uf);

  */
  return 0;
}

