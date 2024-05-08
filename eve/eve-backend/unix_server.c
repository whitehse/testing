#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/eventfd.h>
#include <liburing.h>
#include <fcntl.h>

#include <ev.h>
#include <cbor.h>
#include <cjson/cJSON.h>
#include <eve.h>

#define ERROR(...) do { fprintf(stderr, __VA_ARGS__); exit(1); } while (0)
#define MSG_OUT stdout /* Send info to stdout, change to stderr if you want */
#define DPRINT(x...) printf(x)

#define BUFFER_SIZE 1024

/*
cJSON* cbor_to_cjson(cbor_item_t* item) {
  switch (cbor_typeof(item)) {
    case CBOR_TYPE_UINT:
      return cJSON_CreateNumber(cbor_get_int(item));
    case CBOR_TYPE_NEGINT:
      return cJSON_CreateNumber(-1 - cbor_get_int(item));
    case CBOR_TYPE_BYTESTRING:
      return cJSON_CreateString("Unsupported CBOR item: Bytestring");
    case CBOR_TYPE_STRING:
      if (cbor_string_is_definite(item)) {
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
        if (cbor_isa_string(cbor_map_handle(item)[i].key) &&
            cbor_string_is_definite(cbor_map_handle(item)[i].key)) {
          size_t key_length = cbor_string_length(cbor_map_handle(item)[i].key);
          if (key_length > 127) key_length = 127;
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

    if (desc != NULL)
        printf ("%s:\n", desc);

    for (i = 0; i < len; i++) {

        if ((i % 16) == 0) {
            if (i != 0)
                printf("  %s\n", buff);

            printf("  %04x ", i);
        }

        printf(" %02x", pc[i]);

        if ((pc[i] < 0x20) || (pc[i] > 0x7e)) {
            buff[i % 16] = '.';
        } else {
            buff[i % 16] = pc[i];
        }

        buff[(i % 16) + 1] = '\0';
    }

    while ((i % 16) != 0) {
        printf("   ");
        i++;
    }

    printf("  %s\n", buff);
}
*/

void unix_read_cb(struct ev_loop *loop, struct ev_io *w, int revents){
  char buffer[BUFFER_SIZE];
  ssize_t read;

  if(EV_ERROR & revents) {
    perror("got invalid event");
    return;
  }

  int len;
  struct ucred ucred;

  len = sizeof(struct ucred);
  if (getsockopt(w->fd, SOL_SOCKET, SO_PEERCRED, &ucred, &len) == -1)
    perror("peercred couldn't be retrieved");

  printf("Credentials from SO_PEERCRED: pid=%ld, euid=%ld, egid=%ld\n",
        (long) ucred.pid, (long) ucred.uid, (long) ucred.gid);

  memset(buffer, 0, sizeof(buffer));
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
      perror("Something bad happened");
    }
  }

  printf("received %d bytes from client\n", read);

  hexDump("Received Data", buffer, read);

  struct cbor_load_result result;

  cbor_item_t* item = cbor_load(buffer, read, &result);

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

  //cJSON* cjson_item = cbor_to_cjson(item);
  //char* json_string = cJSON_Print(cjson_item);
  //printf("%s\n", json_string);
  //free(json_string);

  ev_io_stop(loop, w);
  close(w->fd);
  free(w);
}

void unix_accept_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {
  struct sockaddr_un client_addr;
  int client_sd;
  struct ev_io *w_client = (struct ev_io*) malloc (sizeof(struct ev_io));

  if(EV_ERROR & revents) {
    perror("got invalid event");
    return;
  }

  client_sd = accept(watcher->fd, NULL, NULL);

  if (client_sd < 0) {
    perror("accept error");
    return;
  }

  printf("Successfully connected with client.\n");

  ev_io_init(w_client, unix_read_cb, client_sd, EV_READ);
  ev_io_start(loop, w_client);
}

int unix_domain_init(unix_domain_t unix_domain) {
  struct ev_loop *loop = EV_DEFAULT;
  int len;
  int unix_server_fd;
  struct sockaddr_un unix_server;
  char *socket_file_name = "/tmp/unix_socket";

  unix_server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if( unix_server_fd == -1 ) {
    printf("Error on unix socket() call \n");
    return 1;
  }

  unix_server.sun_family = AF_UNIX;
  strcpy(unix_server.sun_path, socket_file_name);
  unlink(unix_server.sun_path);
  len = strlen(unix_server.sun_path) + sizeof(unix_server.sun_family);
  if( bind(unix_server_fd, (struct sockaddr*)&unix_server, len) != 0) {
    printf("Error on binding unix socket \n");
    return 1;
 }

  if( listen(unix_server_fd, 10000) != 0 ) {
    printf("Error on listen call \n");
  }

  struct ev_io unix_accept;
  ev_io_init(&unix_accept, unix_accept_cb, unix_server_fd, EV_READ);
  ev_io_start(loop, &unix_accept);

  ev_run (loop, 0);
}
