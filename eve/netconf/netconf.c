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

#include <ev.h>
#include <cbor.h>
#include <cjson/cJSON.h>
#include <netconf.h>

#define ERROR(...) do { fprintf(stderr, __VA_ARGS__); exit(1); } while (0)
#define MSG_OUT stdout /* Send info to stdout, change to stderr if you want */
#define DPRINT(x...) printf(x)

#define BUFFER_SIZE 1024

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

#define BUFF_SZ   512
char buff[BUFF_SZ + 1];
struct io_uring ring;

static void timeout_cb (EV_P_ ev_timer *w, int revents) {
  fprintf (stderr, "timeout\n");
}

int cbors = 0;

static void socket_cb (struct ev_loop *loop, ev_io *w, int revents) {
  time_t t = time(NULL);
  ssh_t *ssh = w->data;
  struct assh_event_s event;
  assh_status_t err = ASSH_OK;
  ssize_t r;
  int looped = 0;
  int result = assh_event_get(ssh->session, &event, t);
  //printf("socket_cb called, socket %d, socket events %d, assh event %d\n", w->fd, revents, event.id);
  if (event.id < 0 || event.id > 100) {
    sleep(60);
  }
  if (result == 0) {
    ev_io_stop (loop, ssh->socket_watcher_reader);
    ev_io_stop (loop, ssh->socket_watcher_writer);
    goto out;
  }

  struct assh_event_transport_read_s *ter;

  while (result) {
    //printf("Loop %d, ", looped);
    looped = looped + 1;
    switch (event.id) {
      case ASSH_EVENT_READ:
        //printf("EVENT Read Called, ");
        ter = &event.transport.read;
        if (revents & EV_WRITE && ssh->writer_running) {
          // A write (availability) event was received from the socket, via libev,
          // but ASSH is in an ASSH_EVENT_READ state. Stop the ev write watcher and
          // start the ev read watcher
          ev_io_stop(loop, ssh->socket_watcher_writer);
          ssh->writer_running = 0;
          ev_io_start(loop, ssh->socket_watcher_reader);
          ssh->reader_running = 0;
          assh_event_done(ssh->session, &event, err);
          goto out;
        }
        r = read(w->fd, ter->buf.data, ter->buf.size);
        //printf("In READ case, ter->buf.size = %d\n", ter->buf.size);
        switch (r) {
          case -1:
            r = 0;
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
              //break;
              assh_event_done(ssh->session, &event, err);
              goto out;
            }
            break;
          case 0:
            r = -1;
            err = ASSH_ERR_IO;
            //ev_io_stop (loop, w);
          default:
            ter->transferred = r;
            break;
        }
        assh_event_done(ssh->session, &event, err);
        break;

      case ASSH_EVENT_WRITE:
        //printf("EVENT Write Called, ");
        struct assh_event_transport_write_s *tew = &event.transport.write;
        if (revents & EV_READ && ssh->reader_running) {
          // A read event was received from the socket, via libev,
          // but ASSH is in an ASSH_EVENT_WRITE state. Stop the ev read watcher and
          // start the ev write (availability) watcher
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
              //break;
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
        //printf("Wrote %d bytes, ", r);
        assh_event_done(ssh->session, &event, err);
        break;

      case ASSH_EVENT_SESSION_ERROR:
        fprintf(stderr, "SSH error: %s\n", assh_error_str(event.session.error.code));
        // Example: SSH error: IO error
        /* TODO This may not be the right thing to do */
        //assh_session_disconnect(ssh->session, SSH_DISCONNECT_BY_APPLICATION, NULL);
        //close(w->fd);
        //w->data->
        /* */
        assh_event_done(ssh->session, &event, ASSH_OK);
        break;

      case ASSH_EVENT_KEX_HOSTKEY_LOOKUP:
        //printf("Host key lookup called, ", r);
        struct assh_event_kex_hostkey_lookup_s *ek = &event.kex.hostkey_lookup;
        ek->accept = 1;
        assh_event_done(ssh->session, &event, ASSH_OK);
        break;

      case ASSH_EVENT_USERAUTH_CLIENT_BANNER:
        //printf("Client Banner called, ", r);
        struct assh_event_userauth_client_banner_s *eb = &event.userauth_client.banner;
        assert(&event.id == ASSH_EVENT_USERAUTH_CLIENT_BANNER);
        assh_event_done(ssh->session, &event, ASSH_OK);
        break;

      case ASSH_EVENT_USERAUTH_CLIENT_USER:
        //printf("Client User called, ", r);
        struct assh_event_userauth_client_user_s *eu = &event.userauth_client.user;
        //assh_buffer_strset(&eu->username, ssh->user);
        assh_buffer_strset(&eu->username, "support");
        assh_event_done(ssh->session, &event, ASSH_OK);
        break;

      case ASSH_EVENT_USERAUTH_CLIENT_METHODS:
        //printf("Client User methods called, ", r);
        struct assh_event_userauth_client_methods_s *ev = &event.userauth_client.methods;
        assh_buffer_strset(&ev->password, "FiberFAST!!5upporT");
        ev->select = ASSH_USERAUTH_METHOD_PASSWORD;
        //ev->select = ASSH_USERAUTH_METHOD_PUBKEY;
        assh_event_done(ssh->session, &event, ASSH_OK);
        break;

      case ASSH_EVENT_USERAUTH_CLIENT_PWCHANGE:
        //printf("Client PWChange called, ", r);
        assh_event_done(ssh->session, &event, ASSH_OK);
      case ASSH_EVENT_USERAUTH_CLIENT_KEYBOARD:
        //printf("Client Keyboard called, ", r);
        assh_event_done(ssh->session, &event, ASSH_OK);
        break;

      case ASSH_EVENT_SERVICE_START:
        //printf("Service Start called, ", r);
      case ASSH_EVENT_CHANNEL_OPEN:
        //printf("Channel Confirmation open, ", r);
      case ASSH_EVENT_CHANNEL_CONFIRMATION:
        //printf("Channel Confirmation called, ", r);
      case ASSH_EVENT_CHANNEL_FAILURE:
        //printf("Channel Failure called, ", r);
      case ASSH_EVENT_REQUEST_SUCCESS:
        //printf("Request Success called, ", r);
      case ASSH_EVENT_REQUEST_FAILURE:
        //printf("Request Failure called, ", r);
      case ASSH_EVENT_CHANNEL_CLOSE:
        //printf("Channel Close called, ", r);
        asshh_client_event_inter_session(ssh->session, &event, ssh->inter);
        if (ssh->inter->state == ASSH_CLIENT_INTER_ST_CLOSED)
          assh_session_disconnect(ssh->session, SSH_DISCONNECT_BY_APPLICATION, NULL);
        break;

      case ASSH_EVENT_CHANNEL_DATA:
        //printf("state = %d\n", ssh->inter->state);
        //printf("Channel Data called, ", r);
        //printf("Read %d bytes\n", r);
        struct assh_event_channel_data_s *ec = &event.connection.channel_data;
        struct cbor_load_result result;

        assh_status_t err = ASSH_OK;

        //ssize_t r = write(1, ec->data.data, ec->data.size);
        //printf("Buffer size is %d\n", ec->transferred);
        cbors += 1;
        printf("\n%d cbors received.\n", cbors);
        //ter = &event.transport.read;
        cbor_item_t* item = cbor_load(ec->data.data, ec->data.size, &result);
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
        //printf("Wrote %d bytes to stdout, ", r);
        assh_event_done(ssh->session, &event, err);
        break;

      default:
        //printf("Some other event happened: %d\n", event.id);
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

  int number_of_hosts = 1;
  char *hosts[] = {"127.0.0.1"};

  const char *command = argv[2];
  const char *port = "30007";
  //const char *port = "830";

  struct ev_loop *loop = EV_DEFAULT;

  struct addrinfo hints = {
    .ai_family = AF_UNSPEC,
    .ai_socktype = SOCK_STREAM,
  };

  ssh_t ssh[number_of_hosts];
  int sock[number_of_hosts];
  struct addrinfo *servinfo[number_of_hosts];
  struct addrinfo *si[number_of_hosts];
  struct asshh_client_inter_session_s *inter[number_of_hosts];

  signal(SIGPIPE, SIG_IGN);

  //printf("Starting loop\n");
  int status;

  for (int i=0; i<number_of_hosts; i++) {
    //printf("Setting up socket\n");
    sock[i] = -1;
    if (!getaddrinfo(hosts[i], port, &hints, &(servinfo[i]))) {
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

    struct assh_context_s *context[number_of_hosts];
    context[i] = malloc(sizeof(struct assh_context_s));

    if (assh_context_create(&context[i], ASSH_CLIENT,
                            NULL, NULL, NULL, NULL) ||
        assh_service_register_default(context[i]) ||
        assh_algo_register_default(context[i], ASSH_SAFETY_WEAK))
      ERROR("Unable to create an assh context.\n");

    //printf("Finished assh_context_create for %s\n", hosts[i]);

    struct assh_session_s *session[number_of_hosts];
    session[i] = malloc(sizeof(struct assh_session_s));

    if (assh_session_create(context[i], &session[i]))
      ERROR("Unable to create an assh session.\n");

    //printf("Finished assh_session_create for %s\n", hosts[i]);

    enum assh_userauth_methods_e auth_methods =
      ASSH_USERAUTH_METHOD_PUBKEY;

    inter[i] = malloc(sizeof(struct asshh_client_inter_session_s));
    // "pty" doesn't mean anything consequential. It just needs
    // to be a non-NULL value
    asshh_client_init_inter_session(inter[i], command, "pty");
    //asshh_client_init_inter_session(inter[i], command, NULL);
    //inter[i]->state = ASSH_CLIENT_INTER_ST_PTY;
    

    //printf("Finished client_init_inter_session for %s\n", hosts[i]);

    ssh[i].session = session[i];
    ssh[i].inter = inter[i];
    ssh[i].hostname = hosts[i];
    ssh[i].user = user;
    ssh[i].auth_methods = &auth_methods;

    //printf("Finished setting ssh for %s\n", hosts[i]);

    ev_io *socket_watcher[number_of_hosts*2];
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

  //ev_timer timeout_watcher;
  //ev_timer_init (&timeout_watcher, timeout_cb, 0.5, 10.);
  //ev_timer_start (loop, &timeout_watcher);
  //printf("Set timer event\n");

  //ev_run (g.loop, 0);
  ev_run (loop, 0);

  abort();

  printf("Connection closed\n");

  //assh_session_release(session);
}
