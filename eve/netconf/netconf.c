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

int cbors = 0;

int cbs = 0;

static void socket_cb (struct ev_loop *loop, ev_io *w, int revents) {
  if (cbs == 0) {
    puts("socket_cb called");
    cbs = 1;
  }
  time_t t = time(NULL);
  struct ssh *ssh = w->data;
  struct assh_event_s event;
  ssize_t r;
  int looped = 0;
  //if ((revents & EV_WRITE) && ssh->banner_seen == 0) {
  //  goto out;
  //}

  if ((revents & EV_READ) && ssh->banner_seen == 0) {
    char *buffer[1024];
    ssh->banner_seen = 1;
    r = read(w->fd, buffer, 1024);
    printf("%.*s", r, buffer);
    puts("Sending ack");
    r = write(w->fd, "<ack>ok</ack>", 13);
    goto out;
  }
    
  assh_status_t err = ASSH_OK;
  //puts("Calling assh_event_get(ssh->session...");
  int result = assh_event_get(ssh->session, &event, t);
  printf("event.id = %d\n", event.id);
  //puts("Called assh_event_get(ssh->session...");
  //printf("socket_cb called, socket %d, socket events %d, assh event %d\n", w->fd, revents, event.id);
  if (event.id < 0 || event.id > 100) {
    puts("event.id is either less than zero or greater than 100");
    sleep(5);
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
        //hexDump("ssh receive data:", ter->buf.data, ter->buf.size);
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
        assh_buffer_strset(&eu->username, "sysadmin");
        assh_event_done(ssh->session, &event, ASSH_OK);
        break;

      case ASSH_EVENT_USERAUTH_CLIENT_METHODS:
        //printf("Client User methods called, ", r);
        struct assh_event_userauth_client_methods_s *ev = &event.userauth_client.methods;
        assh_buffer_strset(&ev->password, "sysadmin");
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
        //struct cbor_load_result result;

        assh_status_t err = ASSH_OK;

        //ter = &event.transport.read;
        //printf("\n%.*s\n", ec->data.size, ec->data.data);
        //free(buffer);

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

void netconf_listener_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {
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

  printf("Successfully received connection from callhome server.\n");

  struct assh_context_s *context;
  context = malloc(sizeof(struct assh_context_s));

  if (assh_context_create(&context, ASSH_CLIENT,
                          NULL, NULL, NULL, NULL) ||
      assh_service_register_default(context) ||
      assh_algo_register_default(context, ASSH_SAFETY_WEAK))
    ERROR("Unable to create an assh context.\n");

  struct assh_session_s *session;
  session = malloc(sizeof(struct assh_session_s));

  if (assh_session_create(context, &session))
    ERROR("Unable to create an assh session.\n");

  enum assh_userauth_methods_e auth_methods =
    ASSH_USERAUTH_METHOD_PUBKEY;

  struct asshh_client_inter_session_s *inter;
  inter = malloc(sizeof(struct asshh_client_inter_session_s));
  // "pty" doesn't mean anything special. It just needs
  // to be a non-NULL value to force an interactive session
  //asshh_client_init_inter_session(inter, /*command to run*/"show version", "pty");
  asshh_client_init_inter_session(inter, /*command to run*/"show version", NULL);

  struct ssh *ssh;
  ssh = malloc(sizeof(struct ssh));
  ssh->session = session;
  ssh->inter = inter;
  ssh->hostname = '192.168.35.13';
  ssh->user = 'sysadmin';
  ssh->auth_methods = &auth_methods;
  ssh->banner_seen = 0;

  ev_io *writer_watcher;
  ev_io *reader_watcher;
  writer_watcher = malloc(sizeof(ev_io));
  reader_watcher = malloc(sizeof(ev_io));

  ev_io_init (writer_watcher, socket_cb, client_sd, EV_WRITE);
  ssh->socket_watcher_writer = writer_watcher;
  ev_io_init (reader_watcher, socket_cb, client_sd, EV_READ);
  ssh->socket_watcher_reader = reader_watcher;

  writer_watcher->data = ssh;
  reader_watcher->data = ssh;

  ev_io_start (loop, ssh->socket_watcher_reader);
  ssh->reader_running = 1;
  ev_io_start (loop, ssh->socket_watcher_writer);
  ssh->writer_running = 1;
}

static void timeout_cb (EV_P_ ev_timer *w, int revents) {
  fprintf (stderr, "timeout\n");
}

int main(int argc, char **argv) {
  if (assh_deps_init())
    ERROR("initialization error\n");

  if (argc < 3)
    ERROR("usage: ./rexec host 'command'\n");

  const char *user = getenv("USER");
  if (user == NULL)
    ERROR("Unspecified user name\n");

  //int number_of_hosts = 1;
  //char *hosts[] = {"127.0.0.1"};

  //const char *command = argv[2];
  //const char *port = "30007";
  //const char *port = "830";

  struct ev_loop *loop = EV_DEFAULT;

  struct addrinfo hints = {
    .ai_family = AF_UNSPEC,
    .ai_socktype = SOCK_STREAM,
  };

  //int sock[number_of_hosts];
  //struct addrinfo *servinfo[number_of_hosts];
  //struct addrinfo *si[number_of_hosts];

  signal(SIGPIPE, SIG_IGN);

  //printf("Starting loop\n");
  int status;

  int netconf_listener_fd;
  struct sockaddr_in server, client;
  int len;
  int netconf_callhome_port = 4444;
  netconf_listener_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (netconf_listener_fd < 0) {
    perror("Cannot create socket");
    exit(1);
  }
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(netconf_callhome_port);
  len = sizeof(server);
  if (bind(netconf_listener_fd, (struct sockaddr *)&server, len) < 0) {
    perror("Cannot bind sokcet");
    exit(2);
  }
  if (listen(netconf_listener_fd, 100000) < 0) {
    perror("Listen error");
    exit(3);
  }

  struct ev_io w_accept;
  ev_io_init(&w_accept, netconf_listener_cb, netconf_listener_fd, EV_READ);
  ev_io_start(loop, &w_accept);

  //ev_timer timeout_watcher;
  //ev_timer_init (&timeout_watcher, timeout_cb, 0.5, 10.);
  //ev_timer_start (loop, &timeout_watcher);
  //printf("Set timer event\n");

  ev_run (loop, 0);

  abort();

  printf("Connection closed\n");

  //assh_session_release(session);
}
