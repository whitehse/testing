#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

#include <ev.h>
//#define EV_STANDALONE 1
//#include <ev.c>
//#include <cfgpath.h>
//#include <cbor.h>
#include <cjson/cJSON.h>

int counter;

#define EVE_CONNECTION_INIT        1
#define EVE_CONNECTION_ESTABLISHED 2

//cJSON* cbor_to_cjson(cbor_item_t* item) {
//  switch (cbor_typeof(item)) {
//    case CBOR_TYPE_UINT:
//      return cJSON_CreateNumber(cbor_get_int(item));
//    case CBOR_TYPE_NEGINT:
//      return cJSON_CreateNumber(-1 - cbor_get_int(item));
//    case CBOR_TYPE_BYTESTRING:
//      // cJSON only handles null-terminated string -- binary data would have to
//      // be escaped
//      return cJSON_CreateString("Unsupported CBOR item: Bytestring");
//    case CBOR_TYPE_STRING:
//      if (cbor_string_is_definite(item)) {
//        // cJSON only handles null-terminated string
//        char* null_terminated_string = malloc(cbor_string_length(item) + 1);
//        memcpy(null_terminated_string, cbor_string_handle(item),
//               cbor_string_length(item));
//        null_terminated_string[cbor_string_length(item)] = 0;
//        cJSON* result = cJSON_CreateString(null_terminated_string);
//        free(null_terminated_string);
//        return result;
//      }
//      return cJSON_CreateString("Unsupported CBOR item: Chunked string");
//    case CBOR_TYPE_ARRAY: {
//      cJSON* result = cJSON_CreateArray();
//      for (size_t i = 0; i < cbor_array_size(item); i++) {
//        cJSON_AddItemToArray(result, cbor_to_cjson(cbor_array_get(item, i)));
//      }
//      return result;
//    }
//    case CBOR_TYPE_MAP: {
//      cJSON* result = cJSON_CreateObject();
//      for (size_t i = 0; i < cbor_map_size(item); i++) {
//        char* key = malloc(128);
//        snprintf(key, 128, "Surrogate key %zu", i);
//        // JSON only support string keys
//        if (cbor_isa_string(cbor_map_handle(item)[i].key) &&
//            cbor_string_is_definite(cbor_map_handle(item)[i].key)) {
//          size_t key_length = cbor_string_length(cbor_map_handle(item)[i].key);
//          if (key_length > 127) key_length = 127;
//          // Null-terminated madness
//          memcpy(key, cbor_string_handle(cbor_map_handle(item)[i].key),
//                 key_length);
//          key[key_length] = 0;
//        }
//
//        cJSON_AddItemToObject(result, key,
//                              cbor_to_cjson(cbor_map_handle(item)[i].value));
//        free(key);
//      }
//      return result;
//    }
//    case CBOR_TYPE_TAG:
//      return cJSON_CreateString("Unsupported CBOR item: Tag");
//    case CBOR_TYPE_FLOAT_CTRL:
//      if (cbor_float_ctrl_is_ctrl(item)) {
//        if (cbor_is_bool(item)) return cJSON_CreateBool(cbor_get_bool(item));
//        if (cbor_is_null(item)) return cJSON_CreateNull();
//        return cJSON_CreateString("Unsupported CBOR item: Control value");
//      }
//      return cJSON_CreateNumber(cbor_float_get_float(item));
//  }
//
//  return cJSON_CreateNull();
//}

void hex_dump(char *desc, void *addr, int len) {
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

#define BUFFER_SIZE 1024

void echo_read_cb(struct ev_loop *loop, struct ev_io *w, int revents){
  char buffer[BUFFER_SIZE];
  //char buffer[1024];
  ssize_t read;

  //connection_t *connection = w->data;

  //hex_dump(NULL, client_initial_key, 32);

  if(EV_ERROR & revents) {
    perror("got invalid event");
    return;
  }

  memset(buffer, 0, sizeof(buffer));
  //int size = read(w->fd, buffer, sizeof(buffer));
  read = recv(w->fd, buffer, BUFFER_SIZE, 0);
  if ( read < 0 ) {
    perror("read error");
    exit(5);
  }

  if (read == 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      perror("Something weird happened");
      return 0;
    } else {
      puts("Socket was closed");
      ev_io_stop(loop, w);
      close(w->fd);
      free(w);
      return 0;
    }
  }

  counter = counter + 1;
  printf("received %d bytes from client. Packets seen: %d\n", read, counter);
  hex_dump("IPFIX Client: ", buffer, read);
}

void echo_accept_cb(struct ev_loop *loop, struct ev_io *w, int revents) {
  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);
  int client_sd;
  struct ev_io *w_client = (struct ev_io*) malloc (sizeof(struct ev_io));

  if(EV_ERROR & revents) {
    perror("got invalid event");
    return;
  }

  client_sd = accept(w->fd, (struct sockaddr *)&client_addr, &client_len);

  if (client_sd < 0) {
    perror("accept error");
    return;
  }

  printf("Successfully connected with client.\n");

  ev_io_init(w_client, echo_read_cb, client_sd, EV_READ);
  ev_io_start(loop, w_client);
}

int main(int argc, char const *argv[]) {
  struct ev_loop *loop = ev_default_loop(0);
  counter = 0;

  int echo_server_fd;
  struct sockaddr_in server, client;
  int len;
  int echo_port = 1235;
  //echo_server_fd = socket(AF_INET, SOCK_STREAM, 0);
  echo_server_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (echo_server_fd < 0) {
    perror("Cannot create socket");
    exit(1);
  }
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(echo_port);
  len = sizeof(server);
  if (bind(echo_server_fd, (struct sockaddr *)&server, len) < 0) {
    perror("Cannot bind sokcet");
    exit(2);
  }
  //if (listen(echo_server_fd, 100000) < 0) {
  //  perror("Listen error");
  //  exit(3);
  //}

  struct ev_io *w_accept = (struct ev_io*) malloc (sizeof(struct ev_io));
  //ev_io_init(w_accept, echo_accept_cb, echo_server_fd, EV_READ);
  ev_io_init(w_accept, echo_read_cb, echo_server_fd, EV_READ);
  ev_io_start(loop, w_accept);

  ev_run (loop, 0);

  return 0;
}

