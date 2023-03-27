#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h> // exit, atoi
#include <unistd.h> // read, write, close
#include <arpa/inet.h> // sockaddr_in, AF_INET, SOCK_STREAM, INADDR_ANY, socket etc...
#include <string.h> // memset
#include <assert.h>
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
void hexDump(char *desc, void *addr, int len)
{
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

  unsigned char server_pk[crypto_kx_PUBLICKEYBYTES], server_sk[crypto_kx_SECRETKEYBYTES];
  unsigned char server_rx[crypto_kx_SESSIONKEYBYTES], server_tx[crypto_kx_SESSIONKEYBYTES];
  unsigned char client_pk[crypto_kx_PUBLICKEYBYTES];
  /* Generate the client's key pair */
  //crypto_kx_keypair(client_pk, client_sk);

  // server public key
  if ((fptr = fopen("server.pk","rb")) == NULL){
    printf("Error! opening file");

    exit(1);
  }

  fread(&server_pk, crypto_kx_PUBLICKEYBYTES, 1, fptr);
  fclose(fptr); 

  // server secret key
  if ((fptr = fopen("server.sk","rb")) == NULL){
    printf("Error! opening file");

    exit(1);
  }

  fread(&server_sk, crypto_kx_SECRETKEYBYTES, 1, fptr);
  fclose(fptr); 

  // client public key
  if ((fptr = fopen("client.pk","rb")) == NULL){
    printf("Error! opening file");

    exit(1);
  }

  fread(&client_pk, crypto_kx_PUBLICKEYBYTES, 1, fptr);
  fclose(fptr); 

  if (crypto_kx_server_session_keys(server_rx, server_tx,
                                  server_pk, server_sk, client_pk) != 0) {
    /* Suspicious server public key, bail out */
  }

  unsigned char  header[crypto_secretstream_xchacha20poly1305_HEADERBYTES];
  crypto_secretstream_xchacha20poly1305_state state;

  int serverFd, clientFd;
  struct sockaddr_in server, client;
  int len;
  int port = 1234;
  char buffer[1024];
  //if (argc == 2) {
  //  port = atoi(argv[1]);
  //}
  serverFd = socket(AF_INET, SOCK_STREAM, 0);
  if (serverFd < 0) {
    perror("Cannot create socket");
    exit(1);
  }
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(port);
  len = sizeof(server);
  if (bind(serverFd, (struct sockaddr *)&server, len) < 0) {
    perror("Cannot bind sokcet");
    exit(2);
  }
  if (listen(serverFd, 10) < 0) {
    perror("Listen error");
    exit(3);
  }
  while (1) {
    len = sizeof(client);
    printf("waiting for clients\n");
    if ((clientFd = accept(serverFd, (struct sockaddr *)&client, &len)) < 0) {
      perror("accept error");
      exit(4);
    }
    char *client_ip = inet_ntoa(client.sin_addr);
    printf("Accepted new connection from a client %s:%d\n", client_ip, ntohs(client.sin_port));
    memset(buffer, 0, sizeof(buffer));
    int size = read(clientFd, buffer, sizeof(buffer));
    if ( size < 0 ) {
      perror("read error");
      exit(5);
    }

    if (crypto_secretstream_xchacha20poly1305_init_pull(&state, buffer, server_rx) != 0) {
	// incomplete header
	return 1;
    }

    printf("received header, or %d bytes from client\n", size);
    // Length of cyphertext
    printf("Length of HEADER is %d\n", crypto_secretstream_xchacha20poly1305_HEADERBYTES);
    //uint8_t crypted_text_length = ntohs(*(uint8_t*)(buffer+crypto_secretstream_xchacha20poly1305_HEADERBYTES));
    //uint8_t crypted_text_length = (uint8_t*)*(buffer+crypto_secretstream_xchacha20poly1305_HEADERBYTES);
    //printf("The size of the crypted text is %hhu\n", crypted_text_length);
    //uint8_t clear_text_length = ntohs(*(uint8_t*)*(buffer+crypto_secretstream_xchacha20poly1305_HEADERBYTES+1));
    uint8_t clear_text_length = (uint8_t*)*(buffer+crypto_secretstream_xchacha20poly1305_HEADERBYTES);
    uint8_t crypted_text_length = clear_text_length + crypto_secretstream_xchacha20poly1305_ABYTES;
    printf("The size of the clear text is %hhu\n", clear_text_length);
    printf("The size of the crypted text is %hhu\n", crypted_text_length);

    hexDump("Received Data", buffer, size);

    //int message_len = 19;
    //int crypt_buf_len =  19 + crypto_secretstream_xchacha20poly1305_ABYTES;
    //unsigned char crypt_buf[crypt_buf_len];
    unsigned char crypt_buf[crypted_text_length];

    unsigned char tag;
    unsigned long long out_len;

    if (crypto_secretstream_xchacha20poly1305_pull
        (&state, crypt_buf, &out_len, &tag, buffer+crypto_secretstream_xchacha20poly1305_HEADERBYTES+1, crypted_text_length, NULL, 0) != 0) {
      printf("tag = %d\n", tag);
    }
    //assert(tag == 0);

    struct cbor_load_result result;

  cbor_item_t* item = cbor_load(crypt_buf, out_len, &result);
  //free(buffer);

  if (result.error.code != CBOR_ERR_NONE) {
    printf(
        "There was an error while reading the input near byte %zu (read %zu "
        "bytes in total): ",
        result.error.position, result.read);
    switch (result.error.code) {
      case CBOR_ERR_MALFORMATED: {
        printf("Malformed data\n");
        break;
      }
      case CBOR_ERR_MEMERROR: {
        printf("Memory error -- perhaps the input is too large?\n");
        break;
      }
      case CBOR_ERR_NODATA: {
        printf("The input is empty\n");
        break;
      }
      case CBOR_ERR_NOTENOUGHDATA: {
        printf("Data seem to be missing -- is the input complete?\n");
        break;
      }
      case CBOR_ERR_SYNTAXERROR: {
        printf(
            "Syntactically malformed data -- see "
            "https://www.rfc-editor.org/info/std94\n");
        break;
      }
      case CBOR_ERR_NONE: {
        // GCC's cheap dataflow analysis gag
        break;
      }
    }
  } else {
    printf("CBOR data was properly formatted\n");
  }

  cbor_describe(item, stdout);

  cJSON* cjson_item = cbor_to_cjson(item);
  char* json_string = cJSON_Print(cjson_item);
  printf("%s\n", json_string);
  free(json_string);

    //printf("Message: %.19s\n", crypt_buf);
    /*
    printf("received %s from client\n", buffer);
    if (write(clientFd, buffer, size) < 0) {
      perror("write error");
      exit(6);
    }
    */
    close(clientFd);
  }
  close(serverFd);

  return 0;
}

