#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // read, write, close
#include <arpa/inet.h> // sockaddr_in, AF_INET, SOCK_STREAM, INADDR_ANY, socket etc...
#include <string.h> // memset
#include <sys/un.h>
#include <sodium.h>
#include <cbor.h>
#include <cjson/cJSON.h>

cJSON* cbor_to_cjson(cbor_item_t* item) {
  switch (cbor_typeof(item)) {
    case CBOR_TYPE_UINT:
      return cJSON_CreateNumber(cbor_get_int(item));
    case CBOR_TYPE_NEGINT:
      return cJSON_CreateNumber(-1 - cbor_get_int(item));
    case CBOR_TYPE_BYTESTRING:
      // cJSON only handles null-terminated string -- binary data would have to
      // be escaped
      return cJSON_CreateString("Unsupported CBOR item: Bytestring");
    case CBOR_TYPE_STRING:
      if (cbor_string_is_definite(item)) {
        // cJSON only handles null-terminated string
        char* null_terminated_string = malloc(cbor_string_length(item) + 1);
        memcpy(null_terminated_string, cbor_string_handle(item),
               cbor_string_length(item));
        null_terminated_string[cbor_string_length(item)] = 0;
        cJSON* result = cJSON_CreateString(null_terminated_string);
        free(null_terminated_string);
        return result;
      }
      return cJSON_CreateString("Unsupported CBOR item: Chunked string");
    case CBOR_TYPE_ARRAY: {
      cJSON* result = cJSON_CreateArray();
      for (size_t i = 0; i < cbor_array_size(item); i++) {
        cJSON_AddItemToArray(result, cbor_to_cjson(cbor_array_get(item, i)));
      }
      return result;
    }
    case CBOR_TYPE_MAP: {
      cJSON* result = cJSON_CreateObject();
      for (size_t i = 0; i < cbor_map_size(item); i++) {
        char* key = malloc(128);
        snprintf(key, 128, "Surrogate key %zu", i);
        // JSON only support string keys
        if (cbor_isa_string(cbor_map_handle(item)[i].key) &&
            cbor_string_is_definite(cbor_map_handle(item)[i].key)) {
          size_t key_length = cbor_string_length(cbor_map_handle(item)[i].key);
          if (key_length > 127) key_length = 127;
          // Null-terminated madness
          memcpy(key, cbor_string_handle(cbor_map_handle(item)[i].key),
                 key_length);
          key[key_length] = 0;
        }

        cJSON_AddItemToObject(result, key,
                              cbor_to_cjson(cbor_map_handle(item)[i].value));
        free(key);
      }
      return result;
    }
    case CBOR_TYPE_TAG:
      return cJSON_CreateString("Unsupported CBOR item: Tag");
    case CBOR_TYPE_FLOAT_CTRL:
      if (cbor_float_ctrl_is_ctrl(item)) {
        if (cbor_is_bool(item)) return cJSON_CreateBool(cbor_get_bool(item));
        if (cbor_is_null(item)) return cJSON_CreateNull();
        return cJSON_CreateString("Unsupported CBOR item: Control value");
      }
      return cJSON_CreateNumber(cbor_float_get_float(item));
  }

  return cJSON_CreateNull();
}

void hexDump(char *desc, void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    // Output description if given.
    if (desc != NULL)
        printf ("%s:\n", desc);

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf("  %s\n", buff);

            // Output the offset.
            printf("  %04x ", i);
        }

        // Now the hex code for the specific character.
        printf(" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e)) {
            buff[i % 16] = '.';
        } else {
            buff[i % 16] = pc[i];
        }

        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        printf("   ");
        i++;
    }

    // And print the final ASCII bit.
    printf("  %s\n", buff);
}

int main(int argc, char const *argv[]) {
  FILE *fptr;

/*
  unsigned char client_pk[crypto_kx_PUBLICKEYBYTES], client_sk[crypto_kx_SECRETKEYBYTES];
  unsigned char client_rx[crypto_kx_SESSIONKEYBYTES], client_tx[crypto_kx_SESSIONKEYBYTES];
  unsigned char server_pk[crypto_kx_PUBLICKEYBYTES];

  // client public key
  if ((fptr = fopen("client.pk","rb")) == NULL){
    printf("Error! opening file");

    exit(1);
  }

  fread(&client_pk, crypto_kx_PUBLICKEYBYTES, 1, fptr);
  fclose(fptr); 

  // client secret key
  if ((fptr = fopen("client.sk","rb")) == NULL){
    printf("Error! opening file");

    exit(1);
  }

  fread(&client_sk, crypto_kx_SECRETKEYBYTES, 1, fptr);
  fclose(fptr); 

  // server public key
  if ((fptr = fopen("server.pk","rb")) == NULL){
    printf("Error! opening file");

    exit(1);
  }

  fread(&server_pk, crypto_kx_PUBLICKEYBYTES, 1, fptr);
  fclose(fptr); 

  if (crypto_kx_client_session_keys(client_rx, client_tx,
                                  client_pk, client_sk, server_pk) != 0) {
    return 1;
  }

  int r;

  crypto_secretstream_xchacha20poly1305_state state;
  unsigned char header[crypto_secretstream_xchacha20poly1305_HEADERBYTES];

  r = crypto_secretstream_xchacha20poly1305_init_push(&state, header, client_tx);
*/

  int unix_server_fd;
  struct sockaddr_un unix_server;
  char *socket_file_name = "/tmp/unix_socket";
  int ret;
  //int len;
  //int port = 1234;
  
  //char *server_ip = "127.0.0.1";
  //char *buffer = "hello server";

  unix_server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (unix_server_fd < 0) {
    perror("Cannot create socket");
    exit(1);
  }
  unix_server.sun_family = AF_UNIX;
  strcpy(unix_server.sun_path, socket_file_name);
  //unlink(unix_server.sun_path);
  //len = strlen(unix_server.sun_path) + sizeof(unix_server.sun_family);
  //if( bind(unix_server_fd, (struct sockaddr*)&unix_server, len) != 0) {
  //  printf("Error on binding unix socket \n");
  //  return 1;
  //}
  ret = connect(unix_server_fd, &unix_server, sizeof(struct sockaddr_un));

  if (ret == -1) {
    perror("The server is down");
    exit(EXIT_FAILURE);
  }

  cbor_item_t* root = cbor_new_definite_map(2);
  /* Add the content */
  bool success = cbor_map_add(
      root, (struct cbor_pair){
                .key = cbor_move(cbor_build_string("Is CBOR awesome?")),
                .value = cbor_move(cbor_build_bool(true))});
  success &= cbor_map_add(
      root, (struct cbor_pair){
                .key = cbor_move(cbor_build_uint8(42)),
                .value = cbor_move(cbor_build_string("Is the answer"))});
  if (!success) return 1;
  /* Output: `length` bytes of data in the `buffer` */
  unsigned char* cbuffer;
  size_t cbuffer_size;
  cbor_serialize_alloc(root, &cbuffer, &cbuffer_size);
  uint8_t cbuffer_len = (uint8_t)cbuffer_size;

/*
  int tag = crypto_secretstream_xchacha20poly1305_TAG_MESSAGE;
  uint8_t crypt_buf_len =  cbuffer_size + crypto_secretstream_xchacha20poly1305_ABYTES;
  unsigned char crypt_buf[crypt_buf_len];

  crypto_secretstream_xchacha20poly1305_push(&state, crypt_buf, NULL, cbuffer, cbuffer_len,
                                                   NULL, 0, tag);

  char buf[100];
  memcpy(buf, header, crypto_secretstream_xchacha20poly1305_HEADERBYTES);
  memcpy(buf+crypto_secretstream_xchacha20poly1305_HEADERBYTES, &cbuffer_len, 1);
  memcpy(buf+crypto_secretstream_xchacha20poly1305_HEADERBYTES+1, crypt_buf, crypt_buf_len);
  int write_length = crypto_secretstream_xchacha20poly1305_HEADERBYTES+1+crypt_buf_len;
  printf("buffer size is %d\n", write_length);
*/

  ret = write(unix_server_fd, cbuffer, cbuffer_len);
  if (ret == -1) {
    perror("write error");
  }

  free(cbuffer);
  cbor_decref(&root);

  close(unix_server_fd);
  return 0;
}
