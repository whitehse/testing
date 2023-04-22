#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/unistd.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <ev.h>
#include <eio.h>
#include <libwebsockets.h>
#include <sodium.h>
#include <confuse.h>
#include <lmdb.h>

static ev_idle repeat_watcher;
static ev_async ready_watcher;
 
int sockfd;
int number_of_sockets = 0;
int sockets[10];

/* idle watcher callback, only used when eio_poll */
/* didn't handle all results in one call */
static void repeat (EV_P_ ev_idle *w, int revents) {
  if (eio_poll () != -1)
    ev_idle_stop (EV_A_ w);
}

/* eio has some results, process them */
static void ready (EV_P_ ev_async *w, int revents) {
  if (eio_poll () == -1)
    ev_idle_start (EV_A_ &repeat_watcher);
}

/* Read client message */
void read_cb(struct ev_loop *loop, struct ev_io *watcher, int revents){
  char buffer[1024];
  ssize_t read;

  if(EV_ERROR & revents)
  {
    perror("got invalid event");
    return;
  }

  // Receive message from client socket
  read = recv(watcher->fd, buffer, 1024, 0);

  if(read < 0)
  {
    perror("read error");
    return;
  }

  if(read == 0)
  {
    // Stop and free watchet if client socket is closing
    ev_io_stop(loop,watcher);
    free(watcher);
    perror("peer might closing");
    return;
  }
  else
  {
    printf("message:%s",buffer);
  }

  // Send message bach to the client
  /* send(watcher->fd, buffer, read, 0); */
  bzero(buffer, read);
}

void accept_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {
  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);
  int client_sd;
  struct ev_io *w_client = (struct ev_io*) malloc (sizeof(struct ev_io));

  if(EV_ERROR & revents)
  {
    perror("got invalid event");
    return;
  }

  // Accept client request
  client_sd = accept(watcher->fd, (struct sockaddr *)&client_addr, &client_len);

  if (client_sd < 0)
  {
    perror("accept error");
    return;
  }

  sockets[number_of_sockets] = client_sd;
  number_of_sockets += 1;

  puts("Accepted connection");

  // Initialize and start watcher to read client requests
  ev_io_init(w_client, read_cb, client_sd, EV_READ);
  ev_io_start(loop, w_client);
}

void sig_cb(struct ev_loop *loop, struct ev_signal *watcher, int revents) {
  puts("SIGINT called");
  for (int i=0; i<number_of_sockets; i++) {
    close(sockets[i]);
  }
  close(sockfd);
  abort();
}

int child_stdin; 
int child_stdout;
int child_stderr;

/*
int rc;
MDB_env *env;
MDB_dbi dbi;
MDB_val key, data;
MDB_txn *txn;
MDB_cursor *cursor;
char sval[32];
*/

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

//int fork_as(int uid, int gid, char **env, char *command) {
int exec_as(int uid, int gid, int *child_stdin, int *child_stdout, int *child_stderr, char *command) {
  int in[2];
  int out[2];
  int err[2];
  int rc;
  int pid;
  //int status;

  rc = pipe2(in, O_NONBLOCK);
  //rc = pipe2(in, O_DIRECT);
  if (rc<0) goto error_in;

  rc = pipe2(out, O_NONBLOCK);
  //rc = pipe2(out, O_DIRECT);
  if (rc<0) goto error_out;

  rc = pipe2(err, O_NONBLOCK);
  //rc = pipe2(err, O_DIRECT);
  if (rc<0) goto error_err;

  pid = fork();

  if (pid > 0) { /* parent */
    close(in[0]);
    close(out[1]);
    close(err[1]);
    *child_stdin = in[1];
    *child_stdout = out[0];
    *child_stderr = err[0];
    return pid;
  } else if (pid == 0) { /* child */
    close(in[1]);
    close(out[0]);
    close(err[0]);
    close(0);
    if(!dup(in[0])) {
      ;
    }
    close(1);
    if(!dup(out[1])) {
      ;
    }
    close(2);
    if(!dup(err[1])) {
      ;
    }

    int flags = fcntl(out[1], F_GETFL, 0);
    fcntl(out[1], F_SETFL, flags | O_NONBLOCK);
    flags = fcntl(err[1], F_GETFL, 0);
    fcntl(err[1], F_SETFL, flags | O_NONBLOCK);

    setuid(uid);
    setgid(gid);
    execl("/home/dwhite/eve/netconf.pl", "netconf.pl", NULL);
    _exit(1);
  } else
    goto error_fork;

  return pid;

error_fork:
  close(err[0]);
  close(err[1]);
error_err:
  close(out[0]);
  close(out[1]);
error_out:
  close(in[0]);
  close(in[1]);
error_in:
  return -1;
}

// every watcher type has its own typedef'd struct
// with the name ev_TYPE
ev_io stdin_watcher;
ev_timer timeout_watcher;

char temp_buffer[8192];

// all watcher callbacks have a similar signature
// this callback is called when data is readable on stdin
static void
child_stdout_cb (EV_P_ ev_io *w, int revents)
{
  int nbytes;
  ioctl(child_stdout, FIONREAD, &nbytes);
  printf ("There are %d available bytes on child_stdout\n", nbytes);

  //int flags = fcntl(child_stdout, F_GETFL, 0);
  //fcntl(child_stdout, F_SETFL, flags | O_NONBLOCK);

  //ssize_t amount_read = read (child_stdout, temp_buffer, 64*1024);
  ssize_t amount_read = read (child_stdout, temp_buffer, 8192);
  //printf("%d read, data:%s\n", amount_read, temp_buffer);
  //printf("%d read, data:\n%.*s", amount_read, amount_read, temp_buffer);

  char *protobuf_ptr = temp_buffer;
  protobuf_ptr += 8;

  unsigned int type = temp_buffer[0];
  unsigned int length = temp_buffer[4];

  //printf ("Type = %u, length = %u\n", type, length);

  hexDump ("ont-missing", temp_buffer, nbytes);
}

// another callback, this time for a time-out
static void
timeout_cb (EV_P_ ev_timer *w, int revents)
{
  puts("Timer triggered");
  //// this causes the innermost ev_run to stop iterating
  //ev_break (EV_A_ EVBREAK_ONE);
}

int
main (void) {

    struct passwd *p;

    if ((p = getpwnam("dwhite")) == NULL) {
        perror("user dwhite not found");
        return EXIT_FAILURE;
    }
    printf("User dwhite has a uid of %d\n", (int) p->pw_uid);

    struct group *g;

    if ((g = getgrnam("dwhite")) == NULL) {
        perror("group dwhite not found");
        return EXIT_FAILURE;
    }
    printf("Group dwhite has a gid of %d\n", (int) g->gr_gid);

    //exec_as(p->pw_uid, g->gr_gid, &child_stdin, &child_stdout, &child_stderr, "/usr/bin/perl --version");
    //exec_as(p->pw_uid, g->gr_gid, &child_stdin, &child_stdout, &child_stderr, "/home/dwhite/eve/netconf.pl");
    //exec_as(0, 0, &child_stdin, &child_stdout, &child_stderr, "/home/dwhite/eve/netconf.pl");

    //int flags = fcntl(child_stdout, F_GETFL, 0);
    //fcntl(child_stdout, F_SETFL, flags | O_NONBLOCK);

    //char temp_buffer[10];
    //read (child_stdout, temp_buffer, 10);
    //printf ("Buffer = %s", temp_buffer);

    cfg_opt_t system_opts[] = {
        CFG_STR("daemon_uid", "eve", CFGF_NONE), 
        CFG_STR("daemon_gid", "eve", CFGF_NONE), 
        CFG_STR("perl_bin", "/usr/bin/perl", CFGF_NONE), 
        CFG_STR("perldoc_bin", "/usr/bin/perldoc", CFGF_NONE), 
        CFG_STR("perlscript_location", "", CFGF_NONE), 
        CFG_END()
    };

    cfg_opt_t e7_opts[] = {
        CFG_STR("host", "", CFGF_NONE), 
        CFG_END()
    };

    cfg_opt_t e7_daemon_opts[] = {
        CFG_STR("daemon_uid", "e7", CFGF_NONE), 
        CFG_STR("daemon_gid", "e7", CFGF_NONE), 
        CFG_STR("socket", "/var/run/eve/e7/", CFGF_NONE), 
        CFG_STR("connection_type", "ssh", CFGF_NONE), 
        CFG_INT("connection_port", 830, CFGF_NONE), 
        CFG_STR("connection_user", "ro", CFGF_NONE), 
        CFG_STR("connection_pass", "secret", CFGF_NONE), 
        CFG_SEC("e7", e7_opts, CFGF_TITLE | CFGF_MULTI),
        CFG_END()
    };

    cfg_opt_t mx_opts[] = {
        CFG_STR("host", "", CFGF_NONE), 
        CFG_END()
    };

    cfg_opt_t mx_daemon_opts[] = {
        CFG_STR("daemon_uid", "mx", CFGF_NONE), 
        CFG_STR("daemon_gid", "mx", CFGF_NONE), 
        CFG_STR("socket", "/var/run/eve/mx/", CFGF_NONE), 
        CFG_STR("connection_type", "ssh", CFGF_NONE), 
        CFG_INT("connection_port", 830, CFGF_NONE), 
        CFG_STR("connection_user", "ro", CFGF_NONE), 
        CFG_STR("connection_pass", "secret", CFGF_NONE), 
        CFG_SEC("mx", mx_opts, CFGF_TITLE | CFGF_MULTI),
        CFG_END()
    };

    cfg_opt_t slackbot_daemon_opts[] = {
        CFG_STR("daemon_uid", "slackbot", CFGF_NONE), 
        CFG_STR("daemon_gid", "slackbot", CFGF_NONE), 
        CFG_STR("socket", "/var/run/eve/mx/", CFGF_NONE), 
        CFG_STR("bot_token", "", CFGF_NONE), 
        CFG_END()
    };

    cfg_opt_t webserver_daemon_opts[] = {
        CFG_STR("daemon_uid", "www-data", CFGF_NONE), 
        CFG_STR("daemon_gid", "www-data", CFGF_NONE), 
        CFG_STR("listen_address", "localhost", CFGF_NONE), 
        CFG_INT("listen_port", 443, CFGF_NONE), 
        CFG_STR("tls_cert", "", CFGF_NONE), 
        CFG_STR("tls_key", "", CFGF_NONE), 
        CFG_STR("socket", "/var/run/eve/mx/", CFGF_NONE), 
        CFG_END()
    };

    cfg_opt_t tui_daemon_opts[] = {
        CFG_STR("daemon_uid", "tui", CFGF_NONE), 
        CFG_STR("daemon_gid", "tui", CFGF_NONE), 
        CFG_STR("listen_socket", "/var/run/eve/eve_client", CFGF_NONE), 
        CFG_STR("socket", "/var/run/eve/tui/", CFGF_NONE), 
        CFG_END()
    };

    cfg_opt_t ipfix_daemon_opts[] = {
        CFG_STR("daemon_uid", "ipfix", CFGF_NONE), 
        CFG_STR("daemon_gid", "ipfix", CFGF_NONE), 
        CFG_STR("listen_address", "0.0.0.0", CFGF_NONE), 
        CFG_INT("listen_port", 4739, CFGF_NONE), 
        CFG_STR("socket", "/var/run/eve/ipfix/", CFGF_NONE), 
        CFG_END()
    };

    cfg_opt_t syslog_daemon_opts[] = {
        CFG_STR("daemon_uid", "syslog", CFGF_NONE), 
        CFG_STR("daemon_gid", "syslog", CFGF_NONE), 
        CFG_STR("listen_address", "0.0.0.0", CFGF_NONE), 
        CFG_INT("listen_port", 514, CFGF_NONE), 
        CFG_STR("socket", "/var/run/eve/syslog/", CFGF_NONE), 
        CFG_END()
    };

    cfg_opt_t snmptrap_daemon_opts[] = {
        CFG_STR("daemon_uid", "syslog", CFGF_NONE), 
        CFG_STR("daemon_gid", "syslog", CFGF_NONE), 
        CFG_STR("listen_address", "0.0.0.0", CFGF_NONE), 
        CFG_INT("listen_port", 162, CFGF_NONE), 
        CFG_STR("socket", "/var/run/eve/snmptrap/", CFGF_NONE), 
        CFG_END()
    };

    cfg_opt_t flowspec_daemon_opts[] = {
        CFG_STR("daemon_uid", "syslog", CFGF_NONE), 
        CFG_STR("daemon_gid", "syslog", CFGF_NONE), 
        CFG_STR("socket", "/var/run/eve/flowspec/", CFGF_NONE), 
        CFG_END()
    };

    cfg_opt_t analysisbot_daemon_opts[] = {
        CFG_STR("daemon_uid", "syslog", CFGF_NONE), 
        CFG_STR("daemon_gid", "syslog", CFGF_NONE), 
        CFG_STR("socket", "/var/run/eve/analysisbot/", CFGF_NONE), 
        CFG_END()
    };

    cfg_opt_t smx_api_opts[] = {
        CFG_STR("daemon_uid", "syslog", CFGF_NONE), 
        CFG_STR("daemon_gid", "syslog", CFGF_NONE), 
        CFG_STR("socket", "/var/run/eve/smx/", CFGF_NONE), 
        CFG_END()
    };

    cfg_opt_t calixcloud_api_opts[] = {
        CFG_STR("daemon_uid", "syslog", CFGF_NONE), 
        CFG_STR("daemon_gid", "syslog", CFGF_NONE), 
        CFG_STR("socket", "/var/run/eve/csc/", CFGF_NONE), 
        CFG_END()
    };

    cfg_opt_t alianza_api_opts[] = {
        CFG_STR("daemon_uid", "syslog", CFGF_NONE), 
        CFG_STR("daemon_gid", "syslog", CFGF_NONE), 
        CFG_STR("socket", "/var/run/eve/alianza/", CFGF_NONE), 
        CFG_END()
    };

    cfg_opt_t opts[] = {
        CFG_SEC("system", system_opts, CFGF_NONE),
        CFG_SEC("e7_daemon", e7_daemon_opts, CFGF_NONE),
        CFG_SEC("mx_daemon", mx_daemon_opts, CFGF_NONE),
        CFG_SEC("slackbot_daemon", slackbot_daemon_opts, CFGF_NONE),
        CFG_SEC("webserver_daemon", webserver_daemon_opts, CFGF_NONE),
        CFG_SEC("tui_daemon", tui_daemon_opts, CFGF_NONE),
        CFG_SEC("ipfix_daemon", ipfix_daemon_opts, CFGF_NONE),
        CFG_SEC("syslog_daemon", syslog_daemon_opts, CFGF_NONE),
        CFG_SEC("snmptrap_daemon", snmptrap_daemon_opts, CFGF_NONE),
        CFG_SEC("flowspec_daemon", flowspec_daemon_opts, CFGF_NONE),
        CFG_SEC("analysisbot_daemon", analysisbot_daemon_opts, CFGF_NONE),
        CFG_SEC("smx_api", smx_api_opts, CFGF_NONE),
        CFG_SEC("calixcloud_api", calixcloud_api_opts, CFGF_NONE),
        CFG_SEC("alianza_api", alianza_api_opts, CFGF_NONE),
        CFG_END()
    };

/*
  int repeat;
*/

  cfg_t *cfg;
  cfg = cfg_init(opts, CFGF_NONE);
  if(cfg_parse(cfg, "../eve.conf") == CFG_PARSE_ERROR)
    return 1;   

/*
  printf("Hello");
  for(int i = 0; i < cfg_size(cfg, "targets"); i++) {
    printf(", %s", cfg_getnstr(cfg, "targets", i));
  }
  printf("!\n");
*/

  cfg_free(cfg);

/*
  rc = mdb_env_create(&env);
  rc = mdb_env_open(env, "./testdb", 0, 0664);
  rc = mdb_txn_begin(env, NULL, 0, &txn);
  rc = mdb_open(txn, NULL, 0, &dbi);

  key.mv_size = sizeof(int);
  key.mv_data = sval;
  data.mv_size = sizeof(sval);
  data.mv_data = sval;

  sprintf(sval, "%03x %d foo bar", 32, 3141592);
  rc = mdb_put(txn, dbi, &key, &data, 0);
  rc = mdb_txn_commit(txn);
  if (rc) {
    fprintf(stderr, "mdb_txn_commit: (%d) %s\n", rc, mdb_strerror(rc));
    goto leave;
  }
  rc = mdb_txn_begin(env, NULL, MDB_RDONLY, &txn);
  rc = mdb_cursor_open(txn, dbi, &cursor);
  while ((rc = mdb_cursor_get(cursor, &key, &data, MDB_NEXT)) == 0) {
    printf("key: %p %.*s, data: %p %.*s\n",
    key.mv_data,  (int) key.mv_size,  (char *) key.mv_data,
    data.mv_data, (int) data.mv_size, (char *) data.mv_data);
  }
  mdb_cursor_close(cursor);
  mdb_txn_abort(txn);
*/

leave:
  printf("");
//  mdb_close(env, dbi);
//  mdb_env_close(env);
  int uid = getuid();
  printf("The uid = %d\n", uid);
  if (getuid() == 0) {
      /* process is running as root, drop privileges */
      puts("Process is running as root. Dropping privileges\n");
      if (setgid(1000) != 0) {
          printf("setgid: Unable to drop group privileges: %s", strerror(errno));
          abort();
      }
      if (setuid(1000) != 0) {
          printf("setuid: Unable to drop user privileges: %S", strerror(errno));
          abort();
      }
  }

  ev_default_loop (EVBACKEND_POLL | EVBACKEND_SELECT | EVFLAG_NOENV);

  // use the default event loop unless you have special needs
  struct ev_loop *loop = EV_DEFAULT;

  // initialise an io watcher, then start it
  // this one will watch for stdin to become readable
  ev_io_init (&stdin_watcher, child_stdout_cb, child_stdout, EV_READ);
  ev_io_start (loop, &stdin_watcher);

  // initialise a timer watcher, then start it
  // simple non-repeating 5.5 second timeout
  //ev_timer_init (&timeout_watcher, timeout_cb, 1, 1);
  //ev_timer_start (loop, &timeout_watcher);



  int len;
  struct sockaddr_in servaddr, cli;
  struct ev_io w_accept;
 
  // socket create and verification
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    printf("socket creation failed...\n");
    exit(0);
  }
  else
    printf("Socket successfully created..\n");
  bzero(&servaddr, sizeof(servaddr));

  // assign IP, PORT 
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(1234);
 
  // Binding newly created socket to given IP and verification
  if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
    printf("socket bind failed...\n");
    perror("Bind socket");
    exit(0);
  }
  else
    printf("Socket successfully binded..\n");

  // Now server is ready to listen and verification
  if ((listen(sockfd, 5)) != 0) {
    printf("Listen failed...\n");
    exit(0);
  }
  else
    printf("Server listening..\n");
  len = sizeof(cli);

  ev_io_init(&w_accept, accept_cb, sockfd, EV_READ);
  ev_io_start(loop, &w_accept);

  ev_signal exitsig;
  ev_signal_init (&exitsig, sig_cb, SIGINT);
  ev_signal_start (loop, &exitsig);

  // now wait for events to arrive
  ev_run (loop, 0);

  // break was called, so exit
  return 0;
}
