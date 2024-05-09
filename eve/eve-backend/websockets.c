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
#include <libwebsockets.h>

static const struct lws_http_mount mount = {
        .mountpoint     = "/",        /* mountpoint URL */
        .origin       = "./tmp/mount-origin", /* serve from dir */
        .def        = "index.html",   /* default filename */
        .origin_protocol    = LWSMPRO_FILE,     /* files in a dir */
        .mountpoint_len     = 1,            /* char count */
};

int eve_websockets_init(struct ev_loop *loop) {
  //struct lws_client_connect_info connect_info;
  void *foreign_loops[1] = { loop };
  int BUF_SIZE = 4096;

  struct lws_context_creation_info *info;
  info = (struct lws_context_creation_info*) malloc(sizeof(struct lws_context_creation_info));
  struct lws_context *context;
  const char *p;
  int n = 0, logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE;

  lws_set_log_level(logs, NULL);
  lwsl_user("LWS minimal http server | visit http://localhost:7681\n");

  memset(info, 0, sizeof(struct lws_client_connect_info)); /* otherwise uninitialized garbage */
  info->port = 7681;
  info->mounts = &mount;
  info->error_document_404 = "/404.html";
  info->options =
    LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE | LWS_SERVER_OPTION_LIBEV;
  info->foreign_loops = foreign_loops;

  context = lws_create_context(info);
  if (!context) {
    lwsl_err("lws init failed\n");
    return 1;
  }

  //lws_context_destroy(context);
}
