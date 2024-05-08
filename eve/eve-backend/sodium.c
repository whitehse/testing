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
#include <sodium.h>
#include <cbor.h>
#include <cjson/cJSON.h>
#include <eve.h>

#define ERROR(...) do { fprintf(stderr, __VA_ARGS__); exit(1); } while (0)
#define MSG_OUT stdout /* Send info to stdout, change to stderr if you want */
#define DPRINT(x...) printf(x)

#define BUFFER_SIZE 1024

unsigned char server_pk[crypto_kx_PUBLICKEYBYTES], server_sk[crypto_kx_SECRETKEYBYTES];
unsigned char server_rx[crypto_kx_SESSIONKEYBYTES], server_tx[crypto_kx_SESSIONKEYBYTES];
unsigned char client_pk[crypto_kx_PUBLICKEYBYTES];

void sodium_read_cb(struct ev_loop *loop, struct ev_io *w, int revents){
  char buffer[BUFFER_SIZE];
  //char buffer[1024];
  ssize_t read;
  unsigned char  header[crypto_secretstream_xchacha20poly1305_HEADERBYTES];
  crypto_secretstream_xchacha20poly1305_state state;

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
      perror("Something bad happened");
    }
  }

  if (crypto_secretstream_xchacha20poly1305_init_pull(&state, buffer, server_rx) != 0) {
    return 1;
  }

  printf("received header, or %d bytes from client\n", read);
  printf("Length of HEADER is %d\n", crypto_secretstream_xchacha20poly1305_HEADERBYTES);
  uint8_t clear_text_length = (uint8_t*)*(buffer+crypto_secretstream_xchacha20poly1305_HEADERBYTES);
  uint8_t crypted_text_length = clear_text_length + crypto_secretstream_xchacha20poly1305_ABYTES;
  printf("The size of the clear text is %hhu\n", clear_text_length);
  printf("The size of the crypted text is %hhu\n", crypted_text_length);

  hexDump("Received Data", buffer, read);

  unsigned char crypt_buf[crypted_text_length];

  unsigned char tag;
  unsigned long long out_len;

  if (crypto_secretstream_xchacha20poly1305_pull
    (&state, crypt_buf, &out_len, &tag, buffer+crypto_secretstream_xchacha20poly1305_HEADERBYTES+1, crypted_text_length, NULL, 0) != 0) {
    printf("tag = %d\n", tag);
  }

  struct cbor_load_result result;

  cbor_item_t* item = cbor_load(crypt_buf, out_len, &result);

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

  ev_io_stop(loop, w);
  close(w->fd);
  free(w);
}

void sodium_accept_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {
  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);
  int client_sd;
  struct ev_io *w_client = (struct ev_io*) malloc (sizeof(struct ev_io));

  if(EV_ERROR & revents) {
    perror("got invalid event");
    return;
  }

  client_sd = accept(watcher->fd, (struct sockaddr *)&client_addr, &client_len);

  if (client_sd < 0) {
    perror("accept error");
    return;
  }

  printf("Successfully connected with client.\n");

  ev_io_init(w_client, sodium_read_cb, client_sd, EV_READ);
  ev_io_start(loop, w_client);
}

//int eve_sodium_init(eve_sodium_t *eve_sodium) {
int eve_sodium_init(struct ev_loop *loop) {
  //struct ev_loop *loop = EV_DEFAULT;
  FILE *fptr;

  if ((fptr = fopen("server.pk","rb")) == NULL){
    printf("Error! opening file");
    exit(1);
  }

  fread(&server_pk, crypto_kx_PUBLICKEYBYTES, 1, fptr);
  fclose(fptr);

  if ((fptr = fopen("server.sk","rb")) == NULL){
    printf("Error! opening file");
    exit(1);
  }

  fread(&server_sk, crypto_kx_SECRETKEYBYTES, 1, fptr);
  fclose(fptr);

  if ((fptr = fopen("client.pk","rb")) == NULL){
    printf("Error! opening file");
    exit(1);
  }

  fread(&client_pk, crypto_kx_PUBLICKEYBYTES, 1, fptr);
  fclose(fptr);

  if (crypto_kx_server_session_keys(server_rx, server_tx,
                                  server_pk, server_sk, client_pk) != 0) {
    /* Suspicious server public key, bail out */
    return(1);
  }

  int sodium_server_fd;
  struct sockaddr_in server, client;
  int len;
  int sodium_port = 1234;
  sodium_server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sodium_server_fd < 0) {
    perror("Cannot create socket");
    exit(1);
  }
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(sodium_port);
  len = sizeof(server);
  if (bind(sodium_server_fd, (struct sockaddr *)&server, len) < 0) {
    perror("Cannot bind sokcet");
    exit(2);
  }
  if (listen(sodium_server_fd, 100000) < 0) {
    perror("Listen error");
    exit(3);
  }

  struct ev_io w_accept;
  ev_io_init(&w_accept, sodium_accept_cb, sodium_server_fd, EV_READ);
  ev_io_start(loop, &w_accept);
}
