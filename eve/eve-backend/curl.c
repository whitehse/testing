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

#include <curl/curl.h>
#include <ev.h>
#include <eve.h>

#define ERROR(...) do { fprintf(stderr, __VA_ARGS__); exit(1); } while (0)
#define MSG_OUT stdout /* Send info to stdout, change to stderr if you want */
#define DPRINT(x...) printf(x)

#define BUFFER_SIZE 1024

typedef struct _global_info {
  struct ev_loop *loop;
  //struct ev_io fifo_event;
  struct ev_timer timer_event;
  CURLM *multi;
  int still_running;
  FILE *input;
} global_info_t;

global_info_t g;

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
 
}

//int eve_curl_init(eve_curl_t *eve_curl) {
int eve_curl_init(struct ev_loop *loop) {
  //struct ev_loop *loop = EV_DEFAULT;

  //memset(g, 0, sizeof(global_info_t));
  g.loop = loop;
  g.multi = curl_multi_init();
  ev_timer_init(&g.timer_event, curl_timer_cb, 0., 0.);
  g.timer_event.data = &g;
  curl_multi_setopt(g.multi, CURLMOPT_SOCKETFUNCTION, curl_sock_cb);
  curl_multi_setopt(g.multi, CURLMOPT_SOCKETDATA, &g);
  curl_multi_setopt(g.multi, CURLMOPT_TIMERFUNCTION, curl_multi_timer_cb);
  curl_multi_setopt(g.multi, CURLMOPT_TIMERDATA, &g);
  curl_new_conn("http://localhost:7681", &g);
}
