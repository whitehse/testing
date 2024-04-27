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

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <ev.h>
#include <eio.h>
#include <netconf.h>

#define ERROR(...) do { fprintf(stderr, __VA_ARGS__); exit(1); } while (0)

ev_io stdin_watcher;
ev_io stdout_watcher;
ev_timer timeout_watcher;

static void timeout_cb (EV_P_ ev_timer *w, int revents) {
  fprintf (stderr, "timeout\n");
}

static void socket_cb (struct ev_loop *loop, ev_io *w, int revents) {
  time_t t = time(NULL);
  ssh_t *ssh = w->data;
  struct assh_event_s event;

  while (assh_event_get(ssh->session, &event, t)) {
    switch (event.id) {
      case ASSH_EVENT_READ:
      case ASSH_EVENT_WRITE:
	      asshh_fd_event(ssh->session, &event, w->fd);
	      break;

      case ASSH_EVENT_SESSION_ERROR:
        fprintf(stderr, "SSH error: %s\n",
        assh_error_str(event.session.error.code));
        assh_event_done(ssh->session, &event, ASSH_OK);
        break;

      case ASSH_EVENT_KEX_HOSTKEY_LOOKUP:
        asshh_client_event_hk_lookup(ssh->session, stderr, stdin, ssh->hostname, &event);
        break;

      case ASSH_EVENT_USERAUTH_CLIENT_BANNER:
      case ASSH_EVENT_USERAUTH_CLIENT_USER:
      case ASSH_EVENT_USERAUTH_CLIENT_METHODS:
      case ASSH_EVENT_USERAUTH_CLIENT_PWCHANGE:
      case ASSH_EVENT_USERAUTH_CLIENT_KEYBOARD:
        asshh_client_event_auth(ssh->session, stderr, stdin, ssh->user, ssh->hostname,
          ssh->auth_methods, asshh_client_user_key_default, &event);

        struct assh_event_userauth_client_user_s *eu = &event.userauth_client.user;

        assh_buffer_strset(&eu->username, ssh->user);
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
        if (r < 0)
          err = ASSH_ERR_IO;
        else
          ec->transferred = r;

        assh_event_done(ssh->session, &event, err);
        break;

      default:
        assh_event_done(ssh->session, &event, ASSH_OK);
	  }
  }
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

  const char *hostname = argv[1];
  const char *command = argv[2];
  const char *port = "22";
  //const char *port = "830";

  struct ev_loop *loop = EV_DEFAULT;
  ssh_t ssh;

  struct addrinfo hints = {
    .ai_family = AF_UNSPEC,
    .ai_socktype = SOCK_STREAM,
  };

  int sock = -1;
  struct addrinfo *servinfo, *si;
  if (!getaddrinfo(hostname, port, &hints, &servinfo))
    {
      for (si = servinfo; si != NULL; si = si->ai_next)
        {
          sock = socket(si->ai_family, si->ai_socktype, si->ai_protocol);
          if (sock < 0)
            continue;

          if (connect(sock, si->ai_addr, si->ai_addrlen))
            {
              close(sock);
              sock = -1;
              continue;
            }

          break;
        }

      freeaddrinfo(servinfo);
    }

  if (sock < 0)
    ERROR("Unable to connect: %s\n", strerror(errno));

  signal(SIGPIPE, SIG_IGN);

  struct assh_context_s *context;

  if (assh_context_create(&context, ASSH_CLIENT,
                          NULL, NULL, NULL, NULL) ||
      assh_service_register_default(context) ||
      assh_algo_register_default(context, ASSH_SAFETY_WEAK))
    ERROR("Unable to create an assh context.\n");

  struct assh_session_s *session;

  if (assh_session_create(context, &session))
    ERROR("Unable to create an assh session.\n");

  enum assh_userauth_methods_e auth_methods =
    ASSH_USERAUTH_METHOD_PASSWORD |
    ASSH_USERAUTH_METHOD_PUBKEY |
    ASSH_USERAUTH_METHOD_KEYBOARD;

  struct asshh_client_inter_session_s inter;
  asshh_client_init_inter_session(&inter, command, NULL);
  //asshh_client_init_inter_session(&inter, NULL, NULL);

  //struct asshh_inter_subsystem_s sub;
  //asshh_inter_init_subsystem(&sub, "netconf");

  struct assh_event_s event;

  ssh.session = session;
  ssh.inter = &inter;
  ssh.hostname = hostname;
  ssh.user = user;
  ssh.auth_methods = &auth_methods;

  ev_io socket_watcher;
  ev_io_init (&socket_watcher, socket_cb, sock, EV_READ|EV_WRITE);
  socket_watcher.data = &ssh;
  ev_io_start (loop, &socket_watcher);

  ev_timer_init (&timeout_watcher, timeout_cb, 5.5, 1.);
  ev_timer_start (loop, &timeout_watcher);

  ev_run (loop, 0);

  abort();

  printf("Connection closed\n");

  assh_session_release(session);
  assh_context_release(context);

  return 0;
}
