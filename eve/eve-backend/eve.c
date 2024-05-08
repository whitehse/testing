#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
//#include <linux/socket.h>
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

#include <assh/assh_session.h>
#include <assh/assh_context.h>
#include <assh/assh_service.h>
#include <assh/assh_userauth_client.h>
#include <assh/assh_compress.h>
#include <assh/assh_connection.h>
#include <assh/helper_key.h>
#include <assh/helper_interactive.h>
#include <assh/helper_client.h>
#include <assh/assh_kex.h>
#include <assh/helper_io.h>
#include <assh/assh_event.h>
#include <assh/assh_algo.h>

#include <curl/curl.h>
#include <ev.h>
#include <libwebsockets.h>
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

typedef struct _global_info {
  struct ev_loop *loop;
  //struct ev_io fifo_event;
  struct ev_timer timer_event;
  CURLM *multi;
  int still_running;
  FILE *input;
} global_info_t;

typedef struct _curl_conn_info {
  CURL *easy;
  char *url;
  global_info_t *global;
  char error[CURL_ERROR_SIZE];
} curl_conn_info_t;

typedef struct _curl_sock_info {
  curl_socket_t sockfd;
  CURL *easy;
  int action;
  long timeout;
  struct ev_io ev;
  int evset;
  global_info_t *global;
} curl_sock_info_t;

/* Check for completed transfers, and remove their easy handles */
static void check_multi_info(global_info_t *g) {
  char *eff_url;
  CURLMsg *msg;
  int msgs_left;
  curl_conn_info_t *conn;
  CURL *easy;
  CURLcode res;
 
  fprintf(MSG_OUT, "REMAINING: %d\n", g->still_running);
  while((msg = curl_multi_info_read(g->multi, &msgs_left))) {
    if(msg->msg == CURLMSG_DONE) {
      easy = msg->easy_handle;
      res = msg->data.result;
      curl_easy_getinfo(easy, CURLINFO_PRIVATE, &conn);
      curl_easy_getinfo(easy, CURLINFO_EFFECTIVE_URL, &eff_url);
      fprintf(MSG_OUT, "DONE: %s => (%d) %s\n", eff_url, res, conn->error);
      curl_multi_remove_handle(g->multi, easy);
      free(conn->url);
      curl_easy_cleanup(easy);
      free(conn);
    }
  }
}
 
/* Die if we get a bad CURLMcode somewhere */
static void mcode_or_die(const char *where, CURLMcode code) {
  if(CURLM_OK != code) {
    const char *s;
    switch(code) {
    case CURLM_BAD_HANDLE:
      s = "CURLM_BAD_HANDLE";
      break;
    case CURLM_BAD_EASY_HANDLE:
      s = "CURLM_BAD_EASY_HANDLE";
      break;
    case CURLM_OUT_OF_MEMORY:
      s = "CURLM_OUT_OF_MEMORY";
      break;
    case CURLM_INTERNAL_ERROR:
      s = "CURLM_INTERNAL_ERROR";
      break;
    case CURLM_UNKNOWN_OPTION:
      s = "CURLM_UNKNOWN_OPTION";
      break;
    case CURLM_LAST:
      s = "CURLM_LAST";
      break;
    default:
      s = "CURLM_unknown";
      break;
    case CURLM_BAD_SOCKET:
      s = "CURLM_BAD_SOCKET";
      fprintf(MSG_OUT, "ERROR: %s returns %s\n", where, s);
      /* ignore this error */
      return;
    }
    fprintf(MSG_OUT, "ERROR: %s returns %s\n", where, s);
    exit(code);
  }
}

/* Called by libev when our timeout expires */
static void curl_timer_cb(EV_P_ ev_timer *w, int revents) {
  DPRINT("%s  w %p revents %i\n", __PRETTY_FUNCTION__, w, revents);
 
  global_info_t *g = (global_info_t *)w->data;
  CURLMcode rc;
 
  rc = curl_multi_socket_action(g->multi, CURL_SOCKET_TIMEOUT, 0,
                                &g->still_running);
  mcode_or_die("timer_cb: curl_multi_socket_action", rc);
  check_multi_info(g);
}

/* Update the event timer after curl_multi library calls */
static int curl_multi_timer_cb(CURLM *multi, long timeout_ms, global_info_t *g) {
  DPRINT("%s %li\n", __PRETTY_FUNCTION__,  timeout_ms);
  ev_timer_stop(g->loop, &g->timer_event);
  if(timeout_ms >= 0) {
    /* -1 means delete, other values are timeout times in milliseconds */
    double  t = timeout_ms / 1000;
    ev_timer_init(&g->timer_event, curl_timer_cb, t, 0.);
    ev_timer_start(g->loop, &g->timer_event);
  }
  return 0;
}

/* Called by libev when we get action on a multi socket */
static void curl_event_cb(EV_P_ struct ev_io *w, int revents) {
  DPRINT("%s  w %p revents %i\n", __PRETTY_FUNCTION__, w, revents);
  global_info_t *g = (global_info_t*) w->data;
  CURLMcode rc;
 
  int action = ((revents & EV_READ) ? CURL_POLL_IN : 0) |
    ((revents & EV_WRITE) ? CURL_POLL_OUT : 0);
  rc = curl_multi_socket_action(g->multi, w->fd, action, &g->still_running);
  mcode_or_die("event_cb: curl_multi_socket_action", rc);
  check_multi_info(g);
  if(g->still_running <= 0) {
    fprintf(MSG_OUT, "last transfer done, kill timeout\n");
    ev_timer_stop(g->loop, &g->timer_event);
  }
}

/* Clean up the curl_sock_info_t structure */
static void curl_remsock(curl_sock_info_t *f, global_info_t *g)
{
  printf("%s  \n", __PRETTY_FUNCTION__);
  if(f) {
    if(f->evset)
      ev_io_stop(g->loop, &f->ev);
    free(f);
  }
}

/* Assign information to a SockInfo structure */
static void curl_setsock(curl_sock_info_t *f, curl_socket_t s, CURL *e, int act,
                    global_info_t *g) {
  printf("%s  \n", __PRETTY_FUNCTION__);
 
  int kind = ((act & CURL_POLL_IN) ? EV_READ : 0) |
             ((act & CURL_POLL_OUT) ? EV_WRITE : 0);
 
  f->sockfd = s;
  f->action = act;
  f->easy = e;
  if(f->evset)
    ev_io_stop(g->loop, &f->ev);
  ev_io_init(&f->ev, curl_event_cb, f->sockfd, kind);
  f->ev.data = g;
  f->evset = 1;
  ev_io_start(g->loop, &f->ev);
}

/* Initialize a new curl_sock_info_t structure */
static void curl_addsock(curl_socket_t s, CURL *easy, int action, global_info_t *g)
{
  curl_sock_info_t *fdp = calloc(1, sizeof(curl_sock_info_t));
 
  fdp->global = g;
  curl_setsock(fdp, s, easy, action, g);
  curl_multi_assign(g->multi, s, fdp);
}

/* CURLMOPT_SOCKETFUNCTION */
static int curl_sock_cb(CURL *e, curl_socket_t s, int what, void *cbp, void *sockp) {
  DPRINT("%s e %p s %i what %i cbp %p sockp %p\n",
         __PRETTY_FUNCTION__, e, s, what, cbp, sockp);
 
  global_info_t *g = (global_info_t*) cbp;
  curl_sock_info_t *fdp = (curl_sock_info_t*) sockp;
  const char *whatstr[]={ "none", "IN", "OUT", "INOUT", "REMOVE"};
 
  fprintf(MSG_OUT,
          "socket callback: s=%d e=%p what=%s ", s, e, whatstr[what]);
  if(what == CURL_POLL_REMOVE) {
    fprintf(MSG_OUT, "\n");
    curl_remsock(fdp, g);
  }
  else {
    if(!fdp) {
      fprintf(MSG_OUT, "Adding data: %s\n", whatstr[what]);
      curl_addsock(s, e, what, g);
    }
    else {
      fprintf(MSG_OUT,
              "Changing action from %s to %s\n",
              whatstr[fdp->action], whatstr[what]);
      curl_setsock(fdp, s, e, what, g);
    }
  }
  return 0;
}

/* CURLOPT_WRITEFUNCTION */
static size_t curl_write_cb(void *ptr, size_t size, size_t nmemb, void *data)
{
  size_t realsize = size * nmemb;
  curl_conn_info_t *conn = (curl_conn_info_t*) data;
  (void)ptr;
  (void)conn;
  return realsize;
}

/* CURLOPT_PROGRESSFUNCTION */
static int curl_prog_cb(void *p, double dltotal, double dlnow, double ult,
                        double uln)
{
  curl_conn_info_t *conn = (curl_conn_info_t *)p;
  (void)ult;
  (void)uln;
 
  fprintf(MSG_OUT, "Progress: %s (%g/%g)\n", conn->url, dlnow, dltotal);
  return 0;
}

/* Create a new easy handle, and add it to the global curl_multi */
static void curl_new_conn(char *url, global_info_t *g)
{
  curl_conn_info_t *conn;
  CURLMcode rc;
 
  conn = calloc(1, sizeof(curl_conn_info_t));
  conn->error[0]='\0';
 
  conn->easy = curl_easy_init();
  if(!conn->easy) {
    fprintf(MSG_OUT, "curl_easy_init() failed, exiting!\n");
    exit(2);
  }
  conn->global = g;
  conn->url = strdup(url);
  curl_easy_setopt(conn->easy, CURLOPT_URL, conn->url);
  curl_easy_setopt(conn->easy, CURLOPT_WRITEFUNCTION, curl_write_cb);
  curl_easy_setopt(conn->easy, CURLOPT_WRITEDATA, conn);
  curl_easy_setopt(conn->easy, CURLOPT_VERBOSE, 1L);
  curl_easy_setopt(conn->easy, CURLOPT_ERRORBUFFER, conn->error);
  curl_easy_setopt(conn->easy, CURLOPT_PRIVATE, conn);
  curl_easy_setopt(conn->easy, CURLOPT_NOPROGRESS, 0L);
  curl_easy_setopt(conn->easy, CURLOPT_PROGRESSFUNCTION, curl_prog_cb);
  curl_easy_setopt(conn->easy, CURLOPT_PROGRESSDATA, conn);
  curl_easy_setopt(conn->easy, CURLOPT_LOW_SPEED_TIME, 3L);
  curl_easy_setopt(conn->easy, CURLOPT_LOW_SPEED_LIMIT, 10L);
 
  fprintf(MSG_OUT,
          "Adding easy %p to multi %p (%s)\n", conn->easy, g->multi, url);
  rc = curl_multi_add_handle(g->multi, conn->easy);
  mcode_or_die("new_conn: curl_multi_add_handle", rc);
 
  /* note that add_handle() sets a timeout to trigger soon so that the
     necessary socket_action() gets called */
}

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
    // incomplete header
    return 1;
  }

  printf("received header, or %d bytes from client\n", read);
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

  hexDump("Received Data", buffer, read);

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

  // Send message back to the client
  //send(watcher->fd, buffer, read, 0);
  //bzero(buffer, read);
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

  // Accept client request
  client_sd = accept(watcher->fd, (struct sockaddr *)&client_addr, &client_len);

  if (client_sd < 0) {
    perror("accept error");
    return;
  }

  //total_clients ++; // Increment total_clients count
  printf("Successfully connected with client.\n");
  //printf("%d client(s) connected.\n", total_clients);

  // Initialize and start watcher to read client requests
  ev_io_init(w_client, sodium_read_cb, client_sd, EV_READ);
  ev_io_start(loop, w_client);
}

#define BUFF_SZ   512
char buff[BUFF_SZ + 1];
struct io_uring ring;

static const struct lws_http_mount mount = {
        .mountpoint     = "/",        /* mountpoint URL */
        .origin       = "./tmp/mount-origin", /* serve from dir */
        .def        = "index.html",   /* default filename */
        .origin_protocol    = LWSMPRO_FILE,     /* files in a dir */
        .mountpoint_len     = 1,            /* char count */
};

int setup_io_uring(int efd) {
  int ret = io_uring_queue_init(8, &ring, 0);
  if (ret) {
    fprintf(stderr, "Unable to setup io_uring: %s\n", strerror(-ret));
    return 1;
  }
  io_uring_register_eventfd(&ring, efd);
  return 0;
}

static void timeout_cb (EV_P_ ev_timer *w, int revents) {
  fprintf (stderr, "timeout\n");
}

static void iou_cb (struct ev_loop *loop, ev_io *w, int revents) {
  puts("iou_cb was called");
  struct io_uring_cqe *cqe;
  eventfd_t v;
  int ret = eventfd_read(w->fd, &v);
  while(ret == 0) {
    int seq_ret = io_uring_wait_cqe(&ring, &cqe);
    if (seq_ret < 0) {
        fprintf(stderr, "Error waiting for completion: %s\n",
                strerror(-ret));
    }
    if (cqe->res < 0) {
      fprintf(stderr, "Error in async operation: %s\n", strerror(-cqe->res));
    }
    printf("Result of the operation: %d\n", cqe->res);
    io_uring_cqe_seen(&ring, cqe);

    printf("Contents read from file:\n%s\n", buff);
    ret = eventfd_read(w->fd, &v);
  }
}

static void socket_cb (struct ev_loop *loop, ev_io *w, int revents) {
  time_t t = time(NULL);
  ssh_t *ssh = w->data;
  struct assh_event_s event;
  assh_status_t err = ASSH_OK;
  ssize_t r;
  int looped = 0;
  int result = assh_event_get(ssh->session, &event, t);
  if (event.id < 0 || event.id > 100) {
    sleep(60);
  }
  if (result == 0) {
    ev_io_stop (loop, ssh->socket_watcher_reader);
    ev_io_stop (loop, ssh->socket_watcher_writer);
    goto out;
  }

  while (result) {
    looped = looped + 1;
    switch (event.id) {
      case ASSH_EVENT_READ:
        struct assh_event_transport_read_s *ter = &event.transport.read;
        if (revents & EV_WRITE && ssh->writer_running) {
          ev_io_stop(loop, ssh->socket_watcher_writer);
          ssh->writer_running = 0;
          ev_io_start(loop, ssh->socket_watcher_reader);
          ssh->reader_running = 0;
          assh_event_done(ssh->session, &event, err);
          goto out;
        }
        r = read(w->fd, ter->buf.data, ter->buf.size);
        switch (r) {
          case -1:
            r = 0;
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
              assh_event_done(ssh->session, &event, err);
              goto out;
            }
            break;
          case 0:
            r = -1;
            err = ASSH_ERR_IO;
          default:
            ter->transferred = r;
            break;
        }
        assh_event_done(ssh->session, &event, err);
        break;

      case ASSH_EVENT_WRITE:
        struct assh_event_transport_write_s *tew = &event.transport.write;
        if (revents & EV_READ && ssh->reader_running) {
          ev_io_stop(loop, ssh->socket_watcher_reader);
          ssh->reader_running = 0;
          ev_io_start(loop, ssh->socket_watcher_writer);
          ssh->writer_running = 1;
          assh_event_done(ssh->session, &event, err);
          goto out;
        }
        r = write(w->fd, tew->buf.data, tew->buf.size);
        switch (r) {
          case -1:
            r = 0;
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
              assh_event_done(ssh->session, &event, err);
              goto out;
            }
            break;
          case 0:
            r = -1;
            err = ASSH_ERR_IO;
            ev_io_stop (loop, w);
          default:
            tew->transferred = r;
            break;
        }
        assh_event_done(ssh->session, &event, err);
        break;

      case ASSH_EVENT_SESSION_ERROR:
        fprintf(stderr, "SSH error: %s\n", assh_error_str(event.session.error.code));
        assh_event_done(ssh->session, &event, ASSH_OK);
        break;

      case ASSH_EVENT_KEX_HOSTKEY_LOOKUP:
        struct assh_event_kex_hostkey_lookup_s *ek = &event.kex.hostkey_lookup;
        ek->accept = 1;
        assh_event_done(ssh->session, &event, ASSH_OK);
        break;

      case ASSH_EVENT_USERAUTH_CLIENT_BANNER:
        struct assh_event_userauth_client_banner_s *eb = &event.userauth_client.banner;
        assert(&event.id == ASSH_EVENT_USERAUTH_CLIENT_BANNER);
        assh_event_done(ssh->session, &event, ASSH_OK);
        break;

      case ASSH_EVENT_USERAUTH_CLIENT_USER:
        struct assh_event_userauth_client_user_s *eu = &event.userauth_client.user;
        assh_buffer_strset(&eu->username, "sysadmin");
        assh_event_done(ssh->session, &event, ASSH_OK);
        break;

      case ASSH_EVENT_USERAUTH_CLIENT_METHODS:
        struct assh_event_userauth_client_methods_s *ev = &event.userauth_client.methods;
        assh_buffer_strset(&ev->password, "sysadmin");
        ev->select = ASSH_USERAUTH_METHOD_PASSWORD;
        assh_event_done(ssh->session, &event, ASSH_OK);
        break;

      case ASSH_EVENT_USERAUTH_CLIENT_PWCHANGE:
        assh_event_done(ssh->session, &event, ASSH_OK);
      case ASSH_EVENT_USERAUTH_CLIENT_KEYBOARD:
        assh_event_done(ssh->session, &event, ASSH_OK);
        break;

      case ASSH_EVENT_SERVICE_START:
      case ASSH_EVENT_CHANNEL_CONFIRMATION:
      case ASSH_EVENT_CHANNEL_FAILURE:
      case ASSH_EVENT_REQUEST_SUCCESS:
      case ASSH_EVENT_REQUEST_FAILURE:
      case ASSH_EVENT_CHANNEL_CLOSE:
        asshh_client_event_inter_session(ssh->session, &event, ssh->inter);
        if (ssh->inter->state == ASSH_CLIENT_INTER_ST_CLOSED)
          assh_session_disconnect(ssh->session, SSH_DISCONNECT_BY_APPLICATION, NULL);
        break;

      case ASSH_EVENT_CHANNEL_DATA:
        struct assh_event_channel_data_s *ec = &event.connection.channel_data;
        assh_status_t err = ASSH_OK;

        ssize_t r = write(1, ec->data.data, ec->data.size);
        switch (r) {
          case -1:
            r = 0;
            if (errno == EAGAIN || errno == EWOULDBLOCK)
              break;
          case 0:
            r = -1;
            err = ASSH_ERR_IO;
          default:
            ec->transferred = r;
        }
        assh_event_done(ssh->session, &event, err);
        break;

      default:
        assh_event_done(ssh->session, &event, ASSH_OK);
    }
    result = assh_event_get(ssh->session, &event, t);
  }
out:
  return;
}

int main(int argc, char **argv) {
  if (assh_deps_init())
    ERROR("initialization error\n");

  if (argc < 3)
    ERROR("usage: ./rexec host 'command'\n");

  const char *user = getenv("USER");
  if (user == NULL)
    ERROR("Unspecified user name\n");

  //int number_of_e7s = 119;
  int number_of_e7s = 1;
  char *e7s[] = {"192.168.41.11", "192.168.41.12", "192.168.41.13", "192.168.41.14", "192.168.41.15", "192.168.41.16", "192.168.39.11", "192.168.39.12", "192.168.39.13", "192.168.39.14", "192.168.39.15", "192.168.40.11", "192.168.37.12", "192.168.37.13", "192.168.37.14", "192.168.37.15", "192.168.37.11", "192.168.37.16", "192.168.37.17", "192.168.37.18", "192.168.38.11", "192.168.38.12", "192.168.38.13", "192.168.38.14", "192.168.38.15", "192.168.38.16", "192.168.38.17", "192.168.38.18", "192.168.38.19", "192.168.38.20", "192.168.34.11", "192.168.34.12", "192.168.34.13", "192.168.34.14", "192.168.35.11", "192.168.35.12", "192.168.35.13", "192.168.35.14", "192.168.35.15", "192.168.33.11", "192.168.33.12", "192.168.33.13", "192.168.33.14", "192.168.36.11", "192.168.36.12", "192.168.36.13", "192.168.42.11", "192.168.42.12", "192.168.42.13", "192.168.42.14", "192.168.42.15", "192.168.42.16", "192.168.42.17", "192.168.42.18", "192.168.44.11", "192.168.44.12", "192.168.44.13", "192.168.44.14", "192.168.44.15", "192.168.44.16", "192.168.44.17", "192.168.44.18", "192.168.47.11", "192.168.47.12", "192.168.47.13", "192.168.47.14", "192.168.47.15", "192.168.51.11", "192.168.51.12", "192.168.51.13", "192.168.51.14", "192.168.51.15", "192.168.51.16", "192.168.51.17", "192.168.51.18", "192.168.49.11", "192.168.49.12", "192.168.46.11", "192.168.46.12", "192.168.46.13", "192.168.46.14", "192.168.46.15", "192.168.46.16", "192.168.46.17", "192.168.46.18", "192.168.45.11", "192.168.45.12", "192.168.45.13", "192.168.45.14", "192.168.48.11", "192.168.48.12", "192.168.48.13", "192.168.48.14", "192.168.48.15", "192.168.48.16", "192.168.50.11", "192.168.50.12", "192.168.50.13", "192.168.50.14", "192.168.50.15", "192.168.52.11", "192.168.52.12", "192.168.52.13", "192.168.53.11", "192.168.53.12", "192.168.53.13", "192.168.53.14", "192.168.53.15", "192.168.53.16", "192.168.54.11", "192.168.54.12", "192.168.54.13", "192.168.54.14", "192.168.54.15", "192.168.43.11", "192.168.43.12", "192.168.43.13", "192.168.43.14", "192.168.43.15"};

  const char *command = argv[2];
  const char *port = "22";
  //const char *port = "830";

  struct ev_loop *loop = EV_DEFAULT;

  struct addrinfo hints = {
    .ai_family = AF_UNSPEC,
    .ai_socktype = SOCK_STREAM,
  };

  ssh_t ssh[number_of_e7s];
  int sock[number_of_e7s];
  struct addrinfo *servinfo[number_of_e7s];
  struct addrinfo *si[number_of_e7s];

  signal(SIGPIPE, SIG_IGN);

  //printf("Starting loop\n");
  int status;

  for (int i=0; i<number_of_e7s; i++) {
    //printf("Setting up socket\n");
    sock[i] = -1;
    if (!getaddrinfo(e7s[i], port, &hints, &(servinfo[i]))) {
      for (si[i] = servinfo[i]; si[i] != NULL; si[i] = si[i]->ai_next) {
        sock[i] = socket(si[i]->ai_family, si[i]->ai_socktype, si[i]->ai_protocol);
        if (sock[i] < 0)
          continue;

        if (connect(sock[i], si[i]->ai_addr, si[i]->ai_addrlen)) {
          close(sock[i]);
          sock[i] = -1;
          continue;
        }

        break;
      }
      freeaddrinfo(servinfo[i]);
    }
    if (sock[i] < 0)
      ERROR("Unable to connect: %s\n", strerror(errno));

    int status = fcntl(sock[i], F_SETFL, fcntl(sock[i], F_GETFL, 0) | O_NONBLOCK);

    if (status == -1) {
      perror("calling fcntl");
    }

    //printf("Socket was opened for %s\n", e7s[i]);

    struct assh_context_s *context[number_of_e7s];
    context[i] = malloc(sizeof(struct assh_context_s));

    if (assh_context_create(&context[i], ASSH_CLIENT,
                            NULL, NULL, NULL, NULL) ||
        assh_service_register_default(context[i]) ||
        assh_algo_register_default(context[i], ASSH_SAFETY_WEAK))
      ERROR("Unable to create an assh context.\n");

    //printf("Finished assh_context_create for %s\n", e7s[i]);

    struct assh_session_s *session[number_of_e7s];
    session[i] = malloc(sizeof(struct assh_session_s));

    if (assh_session_create(context[i], &session[i]))
      ERROR("Unable to create an assh session.\n");

    //printf("Finished assh_session_create for %s\n", e7s[i]);

    enum assh_userauth_methods_e auth_methods =
      ASSH_USERAUTH_METHOD_PASSWORD;/* |
    ASSH_USERAUTH_METHOD_PUBKEY |
    ASSH_USERAUTH_METHOD_KEYBOARD;*/

    struct asshh_client_inter_session_s *inter[number_of_e7s];
    inter[i] = malloc(sizeof(struct asshh_client_inter_session_s));
    asshh_client_init_inter_session(inter[i], command, NULL);

    //printf("Finished client_init_inter_session for %s\n", e7s[i]);

    ssh[i].session = session[i];
    ssh[i].inter = inter[i];
    ssh[i].hostname = e7s[i];
    ssh[i].user = user;
    ssh[i].auth_methods = &auth_methods;

    //printf("Finished setting ssh for %s\n", e7s[i]);

    ev_io *socket_watcher[number_of_e7s*2];
    socket_watcher[i*2] = malloc(sizeof(ev_io));
    socket_watcher[i*2+1] = malloc(sizeof(ev_io));

    ev_io_init (socket_watcher[i*2], socket_cb, sock[i], EV_READ);
    ssh[i].socket_watcher_reader = socket_watcher[i*2];
    ev_io_init (socket_watcher[i*2+1], socket_cb, sock[i], EV_WRITE);
    ssh[i].socket_watcher_writer = socket_watcher[i*2+1];

    socket_watcher[i*2]->data = &ssh[i];
    socket_watcher[i*2+1]->data = &ssh[i];
    ev_io_start (loop, ssh[i].socket_watcher_reader);
    ssh[i].reader_running = 1;
    ev_io_start (loop, ssh[i].socket_watcher_writer);
    ssh[i].writer_running = 1;
  }

  ev_timer timeout_watcher;
  ev_timer_init (&timeout_watcher, timeout_cb, 0.5, 10.);
  ev_timer_start (loop, &timeout_watcher);
  //printf("Set timer event\n");

  int efd;
  efd = eventfd(0, EFD_NONBLOCK);
  if (efd < 0) perror("eventfd");
  setup_io_uring(efd);

  ev_io *iou_watcher;
  iou_watcher = malloc(sizeof(ev_io));
  ev_io_init (iou_watcher, iou_cb, efd, EV_READ);
  ev_io_start (loop, iou_watcher);

  struct io_uring_sqe *sqe;

  sqe = io_uring_get_sqe(&ring);
  if (!sqe) {
    fprintf(stderr, "Could not get SQE.\n");
    return 1;
  }

  int fd = open("/etc/passwd", O_RDONLY);
  io_uring_prep_read(sqe, fd, buff, BUFF_SZ, 0);
  io_uring_submit(&ring);

  struct lws_client_connect_info connect_info;
  void *foreign_loops[1] = { loop };
  int BUF_SIZE = 4096;

  struct lws_context_creation_info info;
  struct lws_context *context;
  const char *p;
  int n = 0, logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE
    /* for LLL_ verbosity above NOTICE to be built into lws,
     * lws must have been configured and built with
     * -DCMAKE_BUILD_TYPE=DEBUG instead of =RELEASE */
    /* | LLL_INFO */ /* | LLL_PARSER */ /* | LLL_HEADER */
    /* | LLL_EXT */ /* | LLL_CLIENT */ /* | LLL_LATENCY */
    /* | LLL_DEBUG */;

  //signal(SIGINT, sigint_handler);

  //if ((p = lws_cmdline_option(argc, argv, "-d")))
  //     logs = atoi(p);

  lws_set_log_level(logs, NULL);
  lwsl_user("LWS minimal http server | visit http://localhost:7681\n");

  memset(&info, 0, sizeof info); /* otherwise uninitialized garbage */
  info.port = 7681;
  info.mounts = &mount;
  info.error_document_404 = "/404.html";
  info.options =
    LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE | LWS_SERVER_OPTION_LIBEV;
  info.foreign_loops = foreign_loops;
  //info.uid = 1000;
  //info.gid = 1000;

  //if (lws_cmdline_option(argc, argv, "--h2-prior-knowledge"))
  //    info.options |= LWS_SERVER_OPTION_H2_PRIOR_KNOWLEDGE;

  context = lws_create_context(&info);
  if (!context) {
    lwsl_err("lws init failed\n");
    return 1;
  }

  global_info_t g;
  //(void)argc;
  //(void)argv;
 
  memset(&g, 0, sizeof(global_info_t));
  g.loop = loop;
 
  //init_fifo(&g);
  g.multi = curl_multi_init();
 
  ev_timer_init(&g.timer_event, curl_timer_cb, 0., 0.);
  g.timer_event.data = &g;
  //g.fifo_event.data = &g;
  curl_multi_setopt(g.multi, CURLMOPT_SOCKETFUNCTION, curl_sock_cb);
  curl_multi_setopt(g.multi, CURLMOPT_SOCKETDATA, &g);
  curl_multi_setopt(g.multi, CURLMOPT_TIMERFUNCTION, curl_multi_timer_cb);
  curl_multi_setopt(g.multi, CURLMOPT_TIMERDATA, &g);

  curl_new_conn("http://localhost:7681", &g);

  FILE *fptr;

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

  unix_domain_t unix_domain; 
  unix_domain_init(&unix_domain);

  ev_run (loop, 0);

  abort();

  printf("Connection closed\n");

  lws_context_destroy(context);
  //assh_session_release(session);
}
