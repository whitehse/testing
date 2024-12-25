#include <stdio.h>
//#include <math.h>
#include <string.h>
#include <sys/queue.h>
#include <ctype.h>
#include <mz.h>
#include "mz_strm.h"
#include <mz_strm_mem.h>
#include "mz_zip.h"

int main(int argc, char* argv[]) {
  void *mem_stream = NULL;
  void *zip_handle = NULL;
  int err;

  mem_stream = mz_stream_mem_create();
  mz_stream_mem_set_grow_size(mem_stream, (128 * 1024));
  mz_stream_open(mem_stream, NULL, MZ_OPEN_MODE_CREATE);

  zip_handle = mz_zip_create();
  err = mz_zip_open(zip_handle, mem_stream, MZ_OPEN_MODE_WRITE);

  /* TODO: unzip operations.. */

  //mz_zip_file *file_info;
  ////ret = mz_zip_locate_entry(zip_handle, "[Content_Types].xml", 0);
  //ret = mz_zip_locate_entry(zip_handle, "xl/worksheets/sheet1.xml", 0);
  ////ret = mz_zip_locate_entry(zip_handle, "xl/workbook.xml", 0);
  //ret = mz_zip_entry_read_open(zip_handle, 0, NULL);
  //if (ret != MZ_OK) {
  //  // Abort. We may not have an implemented abort() in the wasm wasi posix
  //  // glue. If not, we'll need to implement one first.
  //}

  puts("Action");
  mz_zip_file *info = calloc(1, sizeof(mz_zip_file));
  info->version_madeby = MZ_HOST_SYSTEM_UNIX;
  info->compression_method = MZ_COMPRESS_METHOD_DEFLATE;
  info->filename = "test.txt";
  err = mz_zip_entry_write_open(zip_handle, info, MZ_COMPRESS_LEVEL_DEFAULT, 0, NULL);
  if (err) {
    printf("mz_zip_entry_write_open failed with code %d\n", err);
    goto out;
  }
  err = mz_zip_entry_write(zip_handle, "5\n", 2);
  if (err < 0) {
    printf("mz_zip_entry_write failed with code %d\n", err);
    goto out;
  } else {
    printf("Wrote %d byte(s)\n", err);
  }
  err = mz_zip_entry_close(zip_handle);
  if (err) {
    puts("There was an error calling mz_zip_entry_close");
    goto out;
  }

  mz_zip_close(zip_handle);

  FILE *fp;
  fp = fopen("/tmp/output.zip", "w");
  if (fp == NULL) {
    perror("Error opening file: /tmp/output.zip");
    return 1;
  }

  int32_t buffer_length;
  puts("Calling mz_stream_mem_get_buffer_length");
  mz_stream_mem_get_buffer_length(mem_stream, &buffer_length);
  puts("Called mz_stream_mem_get_buffer_length");
  printf("The buffer length is %d\n", buffer_length);
  void* buffer = malloc(buffer_length);
  puts("Calling mz_stream_mem_get_buffer");
  mz_stream_mem_get_buffer(mem_stream, &buffer);
  puts("Called mz_stream_mem_get_buffer");
  puts("Calling fwrite");
  size_t bytes_written = fwrite(buffer, sizeof(char), buffer_length, fp);
  puts("Called fwrite");

  fclose(fp);

out:
  free(info);
  //mz_zip_close(zip_handle);
  //mz_zip_delete(&zip_handle);

  mz_stream_close(mem_stream);
  mz_stream_mem_delete(&mem_stream);

}
