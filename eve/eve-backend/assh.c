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
#include <eve.h>

#define ERROR(...) do { fprintf(stderr, __VA_ARGS__); exit(1); } while (0)
#define MSG_OUT stdout /* Send info to stdout, change to stderr if you want */
#define DPRINT(x...) printf(x)

#define BUFF_SZ   512

static void socket_cb (struct ev_loop *loop, ev_io *w, int revents) {
  time_t t = time(NULL);
  ssh_t *ssh = w->data;
  struct assh_event_s event;
  assh_status_t err = ASSH_OK;
  ssize_t r;
  //int looped = 0;

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
    //looped = looped + 1;
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
        if (ssh->inter->state == ASSH_CLIENT_INTER_ST_CLOSED) {
          assh_session_disconnect(ssh->session, SSH_DISCONNECT_BY_APPLICATION, NULL);
        }
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

//int eve_assh_init(int argc, char **argv) {
int eve_assh_init(struct ev_loop *loop) {
  if (assh_deps_init())
    ERROR("initialization error\n");

  const char *user = getenv("USER");
  if (user == NULL)
    ERROR("Unspecified user name\n");

  //int number_of_e7s = 119;
  int number_of_e7s = 1;
  char *e7s[] = {"192.168.41.11", "192.168.41.12", "192.168.41.13", "192.168.41.14", "192.168.41.15", "192.168.41.16", "192.168.39.11", "192.168.39.12", "192.168.39.13", "192.168.39.14", "192.168.39.15", "192.168.40.11", "192.168.37.12", "192.168.37.13", "192.168.37.14", "192.168.37.15", "192.168.37.11", "192.168.37.16", "192.168.37.17", "192.168.37.18", "192.168.38.11", "192.168.38.12", "192.168.38.13", "192.168.38.14", "192.168.38.15", "192.168.38.16", "192.168.38.17", "192.168.38.18", "192.168.38.19", "192.168.38.20", "192.168.34.11", "192.168.34.12", "192.168.34.13", "192.168.34.14", "192.168.35.11", "192.168.35.12", "192.168.35.13", "192.168.35.14", "192.168.35.15", "192.168.33.11", "192.168.33.12", "192.168.33.13", "192.168.33.14", "192.168.36.11", "192.168.36.12", "192.168.36.13", "192.168.42.11", "192.168.42.12", "192.168.42.13", "192.168.42.14", "192.168.42.15", "192.168.42.16", "192.168.42.17", "192.168.42.18", "192.168.44.11", "192.168.44.12", "192.168.44.13", "192.168.44.14", "192.168.44.15", "192.168.44.16", "192.168.44.17", "192.168.44.18", "192.168.47.11", "192.168.47.12", "192.168.47.13", "192.168.47.14", "192.168.47.15", "192.168.51.11", "192.168.51.12", "192.168.51.13", "192.168.51.14", "192.168.51.15", "192.168.51.16", "192.168.51.17", "192.168.51.18", "192.168.49.11", "192.168.49.12", "192.168.46.11", "192.168.46.12", "192.168.46.13", "192.168.46.14", "192.168.46.15", "192.168.46.16", "192.168.46.17", "192.168.46.18", "192.168.45.11", "192.168.45.12", "192.168.45.13", "192.168.45.14", "192.168.48.11", "192.168.48.12", "192.168.48.13", "192.168.48.14", "192.168.48.15", "192.168.48.16", "192.168.50.11", "192.168.50.12", "192.168.50.13", "192.168.50.14", "192.168.50.15", "192.168.52.11", "192.168.52.12", "192.168.52.13", "192.168.53.11", "192.168.53.12", "192.168.53.13", "192.168.53.14", "192.168.53.15", "192.168.53.16", "192.168.54.11", "192.168.54.12", "192.168.54.13", "192.168.54.14", "192.168.54.15", "192.168.43.11", "192.168.43.12", "192.168.43.13", "192.168.43.14", "192.168.43.15"};

  char *command = (char *)malloc(13*sizeof(char));
  strcpy(command, "show version");
  char *port = "22";
  //const char *port = "830";

  struct addrinfo hints = {
    .ai_family = AF_UNSPEC,
    .ai_socktype = SOCK_STREAM,
  };

  ssh_t *ssh[number_of_e7s];
  int *sock[number_of_e7s];
  struct addrinfo *servinfo[number_of_e7s];
  struct addrinfo *si[number_of_e7s];

  signal(SIGPIPE, SIG_IGN);

  //printf("Starting loop\n");
  int status;

  for (int i=0; i<number_of_e7s; i++) {
    //printf("Setting up socket\n");
    sock[i] = (int *)malloc(sizeof(int));
    *sock[i] = -1;
    if (!getaddrinfo(e7s[i], port, &hints, &(servinfo[i]))) {
      for (si[i] = servinfo[i]; si[i] != NULL; si[i] = si[i]->ai_next) {
        *sock[i] = socket(si[i]->ai_family, si[i]->ai_socktype, si[i]->ai_protocol);
        if (*sock[i] < 0)
          continue;

        if (connect(*sock[i], si[i]->ai_addr, si[i]->ai_addrlen)) {
          close(*sock[i]);
          *sock[i] = -1;
          continue;
        }

        break;
      }
      freeaddrinfo(servinfo[i]);
    }
    if (*sock[i] < 0)
      ERROR("Unable to connect: %s\n", strerror(errno));

    int status = fcntl(*sock[i], F_SETFL, fcntl(*sock[i], F_GETFL, 0) | O_NONBLOCK);

    if (status == -1) {
      perror("calling fcntl");
    }

    struct assh_context_s *context[number_of_e7s];
    context[i] = malloc(sizeof(struct assh_context_s));

    if (assh_context_create(&context[i], ASSH_CLIENT,
                            NULL, NULL, NULL, NULL) ||
        assh_service_register_default(context[i]) ||
        assh_algo_register_default(context[i], ASSH_SAFETY_WEAK))
      ERROR("Unable to create an assh context.\n");

    struct assh_session_s *session[number_of_e7s];
    session[i] = malloc(sizeof(struct assh_session_s));

    if (assh_session_create(context[i], &session[i]))
      ERROR("Unable to create an assh session.\n");

    enum assh_userauth_methods_e auth_methods =
      ASSH_USERAUTH_METHOD_PASSWORD;/* |
    ASSH_USERAUTH_METHOD_PUBKEY |
    ASSH_USERAUTH_METHOD_KEYBOARD;*/

    struct asshh_client_inter_session_s *inter[number_of_e7s];
    inter[i] = malloc(sizeof(struct asshh_client_inter_session_s));
    asshh_client_init_inter_session(inter[i], command, NULL);

    ssh[i] = malloc(sizeof(ssh_t));
    ssh[i]->session = session[i];
    ssh[i]->inter = inter[i];
    ssh[i]->hostname = e7s[i];
    ssh[i]->user = user;
    ssh[i]->auth_methods = &auth_methods;

    ev_io *socket_watcher[number_of_e7s*2];
    socket_watcher[i*2] = malloc(sizeof(ev_io));
    socket_watcher[i*2+1] = malloc(sizeof(ev_io));

    ev_io_init (socket_watcher[i*2], socket_cb, *sock[i], EV_READ);
    ssh[i]->socket_watcher_reader = socket_watcher[i*2];
    ev_io_init (socket_watcher[i*2+1], socket_cb, *sock[i], EV_WRITE);
    ssh[i]->socket_watcher_writer = socket_watcher[i*2+1];

    socket_watcher[i*2]->data = ssh[i];
    socket_watcher[i*2+1]->data = ssh[i];
    ev_io_start (loop, ssh[i]->socket_watcher_reader);
    ssh[i]->reader_running = 1;
    ev_io_start (loop, ssh[i]->socket_watcher_writer);
    ssh[i]->writer_running = 1;
  }
  //assh_session_release(session);
}
