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
/*
        asshh_client_event_hk_lookup(ssh->session, stderr, stdin, ssh->hostname, &event);
*/
        struct assh_event_kex_hostkey_lookup_s *ek = &event.kex.hostkey_lookup;
        ek->accept = 1;
        assh_event_done(ssh->session, &event, ASSH_OK);
        break;

      case ASSH_EVENT_USERAUTH_CLIENT_BANNER:
        struct assh_event_userauth_client_banner_s *eb = &event.userauth_client.banner;
        assert(&event.id == ASSH_EVENT_USERAUTH_CLIENT_BANNER);
        //fprintf(stderr, "%s\n", eb->text);
        assh_event_done(ssh->session, &event, ASSH_OK);
        break;

      case ASSH_EVENT_USERAUTH_CLIENT_USER:
        struct assh_event_userauth_client_user_s *eu = &event.userauth_client.user;
        assh_buffer_strset(&eu->username, ssh->user);
        assh_event_done(ssh->session, &event, ASSH_OK);
        break;

      case ASSH_EVENT_USERAUTH_CLIENT_METHODS:
        struct assh_event_userauth_client_methods_s *ev = &event.userauth_client.methods;
        assh_buffer_strset(&ev->password, "sysadmin");
        ev->select = ASSH_USERAUTH_METHOD_PASSWORD;
        assh_event_done(ssh->session, &event, ASSH_OK);
        break;

      case ASSH_EVENT_USERAUTH_CLIENT_PWCHANGE:
      case ASSH_EVENT_USERAUTH_CLIENT_KEYBOARD:
/*
        asshh_client_event_auth(ssh->session, stderr, stdin, ssh->user, ssh->hostname,
          ssh->auth_methods, asshh_client_user_key_default, &event);

        struct assh_event_userauth_client_user_s *eu = &event.userauth_client.user;
        assh_buffer_strset(&eu->username, ssh->user);
*/
/*
        struct assh_event_kex_hostkey_lookup_s *ek = &event.kex.hostkey_lookup;
        ek->accept = 1;
*/
        assh_event_done(ssh->session, &event, ASSH_OK);
        break;

/*
      case ASSH_EVENT_SERVICE_START:
        assh_bool_t conn = &event.service.start.srv == &assh_service_connection;
        assh_event_done(ssh->session, &event, ASSH_OK);
        if (conn) {
            fprintf(stderr, "conn is true\n");
*/
/*
          //assert(ctx->state == ASSH_CLIENT_INTER_ST_INIT);
          assert(ssh->inter == ASSH_CLIENT_INTER_ST_INIT);

          if (asshh_inter_open_session(ssh->session, ssh->inter->channel))
            fprintf(stderr, "ERR\n");
//            goto err;

          //ASSH_SET_STATE(ctx, state, ASSH_CLIENT_INTER_ST_SESSION);
          ASSH_SET_STATE(ssh->inter, state, ASSH_CLIENT_INTER_ST_SESSION);
*/
/*
        } else {
            fprintf(stderr, "conn is false\n");
        }
        break;
*/
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

  //int number_of_e7s = 5;
  //char *e7s[] = {"192.168.35.11", "192.168.35.12", "192.168.35.13", "192.168.35.14", "192.168.35.15"};

  int number_of_e7s = 1;
  char *e7s[] = {"192.168.35.15"};

  const char *hostname = argv[1];
  //printf("hostname = %s\n", hostname);
  //printf("e7s[0] = %s\n", e7s[0]);
  //const char *hostname2 = "192.168.35.14";
  const char *command = argv[2];
  const char *port = "22";
  //const char *port = "830";

  struct ev_loop *loop = EV_DEFAULT;

  struct addrinfo hints = {
    .ai_family = AF_UNSPEC,
    .ai_socktype = SOCK_STREAM,
  };

  //ssh_t ssh[number_of_e7s];
  ssh_t ssh;
  //ssh_t ssh2;
  //int sock[number_of_e7s];
  int sock;
  //struct addrinfo *servinfo[number_of_e7s];
  //struct addrinfo *si[number_of_e7s];
  struct addrinfo *servinfo;
  struct addrinfo *si;

  signal(SIGPIPE, SIG_IGN);

  printf("Starting loop\n");
  for (int i=0; i<number_of_e7s; i++) {
    printf("Setting up socket\n");
    //sock[i] = -1;
    sock = -1;
    //if (!getaddrinfo(e7s[i], port, &hints, &(servinfo[i]))) {
    if (!getaddrinfo(e7s[i], port, &hints, &servinfo)) {
      //for (si[i] = servinfo[i]; si[i] != NULL; si[i] = si[i]->ai_next) {
      for (si = servinfo; si != NULL; si = si->ai_next) {
        //sock[i] = socket(si[i]->ai_family, si[i]->ai_socktype, si[i]->ai_protocol);
        //sock[i] = socket(si->ai_family, si->ai_socktype, si->ai_protocol);
        sock = socket(si->ai_family, si->ai_socktype, si->ai_protocol);
        //if (sock[i] < 0)
        if (sock < 0)
          continue;

        //if (connect(sock[i], si[i]->ai_addr, si[i]->ai_addrlen)) {
        //if (connect(sock[i], si->ai_addr, si->ai_addrlen)) {
        if (connect(sock, si->ai_addr, si->ai_addrlen)) {
          //close(sock[i]);
          close(sock);
          //sock[i] = -1;
          sock = -1;
          continue;
        }

        break;
      }
      freeaddrinfo(servinfo);
    }
    //if (sock[i] < 0)
    if (sock < 0)
      ERROR("Unable to connect: %s\n", strerror(errno));

    printf("Socket was opened for %s\n", e7s[i]);
/*
  int sock = -1;
  struct addrinfo *servinfo, *si;
  if (!getaddrinfo(hostname, port, &hints, &servinfo)) {
    for (si = servinfo; si != NULL; si = si->ai_next) {
      sock = socket(si->ai_family, si->ai_socktype, si->ai_protocol);
      if (sock < 0)
        continue;

      if (connect(sock, si->ai_addr, si->ai_addrlen)) {
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

  int sock2 = -1;
  struct addrinfo *servinfo2, *si2;
  if (!getaddrinfo(hostname2, port, &hints, &servinfo2)) {
    for (si2 = servinfo2; si2 != NULL; si2 = si2->ai_next) {
      sock2 = socket(si2->ai_family, si2->ai_socktype, si2->ai_protocol);
      if (sock2 < 0)
        continue;

      if (connect(sock2, si2->ai_addr, si2->ai_addrlen)) {
        close(sock2);
        sock2 = -1;
        continue;
      }

      break;
   }

   freeaddrinfo(servinfo2);
  }

  if (sock2 < 0)
    ERROR("Unable to connect: %s\n", strerror(errno));
*/

  //struct assh_context_s *context[number_of_e7s];
  //struct assh_context_s *context2;
  struct assh_context_s *context;

    if (assh_context_create(&context, ASSH_CLIENT,
                            NULL, NULL, NULL, NULL) ||
        assh_service_register_default(context) ||
        assh_algo_register_default(context, ASSH_SAFETY_WEAK))
      ERROR("Unable to create an assh context.\n");

    printf("Finished assh_context_create for %s\n", e7s[i]);
/*
  if (assh_context_create(&context, ASSH_CLIENT,
                          NULL, NULL, NULL, NULL) ||
      assh_service_register_default(context) ||
      assh_algo_register_default(context, ASSH_SAFETY_WEAK))
    ERROR("Unable to create an assh context.\n");

  if (assh_context_create(&context2, ASSH_CLIENT,
                          NULL, NULL, NULL, NULL) ||
      assh_service_register_default(context2) ||
      assh_algo_register_default(context2, ASSH_SAFETY_WEAK))
    ERROR("Unable to create an assh context2.\n");
*/

  //struct assh_session_s *session;
  //struct assh_session_s *session2;
  //struct assh_session_s *session[number_of_e7s];
  struct assh_session_s *session;

    //if (assh_session_create(context[i], &(session[i])))
    //if (assh_session_create(context[i], &session))
    if (assh_session_create(context, &session))
      ERROR("Unable to create an assh session.\n");

    printf("Finished assh_session_create for %s\n", e7s[i]);

  //if (assh_session_create(context2, &session2))
  //  ERROR("Unable to create an assh session2.\n");

    enum assh_userauth_methods_e auth_methods =
      ASSH_USERAUTH_METHOD_PASSWORD;/* |
    ASSH_USERAUTH_METHOD_PUBKEY |
    ASSH_USERAUTH_METHOD_KEYBOARD;*/

//  enum assh_userauth_methods_e auth_methods2 =
//    ASSH_USERAUTH_METHOD_PASSWORD;/* |
//    ASSH_USERAUTH_METHOD_PUBKEY |
//    ASSH_USERAUTH_METHOD_KEYBOARD;*/

    //struct asshh_client_inter_session_s inter[number_of_e7s];
    struct asshh_client_inter_session_s inter;
    //asshh_client_init_inter_session(&inter[i], command, NULL);
    asshh_client_init_inter_session(&inter, command, NULL);

    printf("Finished client_init_inter_session for %s\n", e7s[i]);

    //struct asshh_client_inter_session_s inter2;
    //asshh_client_init_inter_session(&inter2, command, NULL);
    //asshh_client_init_inter_session(&inter2, NULL, NULL);

  //struct asshh_inter_subsystem_s sub;
  //asshh_inter_init_subsystem(&sub, "netconf");

    //struct assh_event_s event[number_of_e7s];
    //struct assh_event_s event2;
    //struct assh_event_s event;

    //ssh[i].session = session[i];
    //ssh[i].inter = &inter[i];
    //ssh[i].hostname = e7s[i];
    //ssh[i].user = user;
    //ssh[i].auth_methods = &auth_methods;
    ssh.session = session;
    ssh.inter = &inter;
    ssh.hostname = e7s[i];
    ssh.user = user;
    ssh.auth_methods = &auth_methods;

    printf("Finished setting ssh for %s\n", e7s[i]);
  //ssh2.session = session2;
  //ssh2.inter = &inter2;
  //ssh2.hostname = hostname2;
  //ssh2.user = user;
  //ssh2.auth_methods = &auth_methods2;

    //ev_io *socket_watcher[number_of_e7s];
    //socket_watcher[i] = malloc(number_of_e7s * sizeof(ev_io));
    //ev_io socket_watcher[number_of_e7s];
    ev_io socket_watcher;

    //ev_io_init (socket_watcher[i], socket_cb, sock[i], EV_READ|EV_WRITE);
    //ev_io_init (&(socket_watcher[i]), socket_cb, sock[i], EV_READ|EV_WRITE);
    //ev_io_init (&socket_watcher, socket_cb, sock[i], EV_READ|EV_WRITE);
    ev_io_init (&socket_watcher, socket_cb, sock, EV_READ|EV_WRITE);

    //socket_watcher[i]->data = &ssh[i];
    //(&(socket_watcher[i]))->data = &(ssh[i]);
    socket_watcher.data = &ssh;

    //ev_io_start (loop, socket_watcher[i]);
    //ev_io_start (loop, &(socket_watcher[i]));
    ev_io_start (loop, &socket_watcher);

  }

  ev_timer_init (&timeout_watcher, timeout_cb, 0.5, 4.);
  ev_timer_start (loop, &timeout_watcher);

  printf("Set timer event\n");
  ev_run (loop, 0);

  abort();

  printf("Connection closed\n");

  //assh_session_release(session);
}
