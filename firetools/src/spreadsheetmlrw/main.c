#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <spreadsheetmlrw.h>

#include <zip.h>
#include <unzip.h>

#define BUF_SIZE 8192
#define MAX_NAMELEN 256

int main() {
  puts("Hello, World!");
  struct ml_type_parse *ml_type_parser;
  ml_type_parser = in_word_set_ml_type ("application/vnd.openxmlformats-officedocument.spreadsheetml.calcChain+xml", 73);
  printf("Name = %s\n", ml_type_parser->name);
  printf("Enum = %d\n", (int)ml_type_parser->ml_type);
  ml_type_parser = in_word_set_ml_type ("application/vnd.openxmlformats-officedocument.spreadsheetml.chartsheet+xml", 74);
  printf("Name = %s\n", ml_type_parser->name);
  printf("Enum = %d\n", (int)ml_type_parser->ml_type);

  unzFile uf = unzOpen64("/tmp/file.xlsx");
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

  return 0;
}
