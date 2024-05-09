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
#include <eve.h>

#define ERROR(...) do { fprintf(stderr, __VA_ARGS__); exit(1); } while (0)
#define MSG_OUT stdout /* Send info to stdout, change to stderr if you want */
#define DPRINT(x...) printf(x)

#define BUFFER_SIZE 1024

#define BUFF_SZ   512
char buff[BUFF_SZ + 1];

int setup_io_uring(int efd, struct io_uring *ring) {
  int ret = io_uring_queue_init(8, ring, 0);
  if (ret) {
    printf("Unable to setup io_uring: %s\n", strerror(-ret));
    return 1;
  }
  io_uring_register_eventfd(ring, efd);
  return 0;
}

static void iou_cb (struct ev_loop *loop, ev_io *w, int revents) {
  struct io_uring_cqe *cqe;
  eventfd_t v;
  struct io_uring *ring = w->data;

  int ret = eventfd_read(w->fd, &v);
  while(ret == 0) {
    int seq_ret = io_uring_wait_cqe(ring, &cqe);
    if (seq_ret < 0) {
        printf("Error waiting for completion: %s\n",
                strerror(-ret));
    }
    if (cqe->res < 0) {
      printf("Error in async operation: %s\n", strerror(-cqe->res));
    }
    printf("Result of the operation: %d\n", cqe->res);
    io_uring_cqe_seen(ring, cqe);

    printf("Contents read from file:\n%s\n", buff);
    ret = eventfd_read(w->fd, &v);
  }
}

int eve_iou_init(struct ev_loop *loop) {
  int efd;
  struct io_uring *ring = (struct io_uring*) malloc(sizeof(struct io_uring));
  efd = eventfd(0, EFD_NONBLOCK);
  if (efd < 0) perror("eventfd");
  setup_io_uring(efd, ring);

  ev_io *iou_watcher;
  iou_watcher = malloc(sizeof(ev_io));
  ev_io_init (iou_watcher, iou_cb, efd, EV_READ);
  iou_watcher->data = ring;
  ev_io_start (loop, iou_watcher);

  struct io_uring_sqe *sqe;

  sqe = io_uring_get_sqe(ring);
  if (!sqe) {
    printf("Could not get SQE.\n");
    return 1;
  }

  int fd = open("/etc/passwd", O_RDONLY);
  io_uring_prep_read(sqe, fd, buff, BUFF_SZ, 0);
  io_uring_submit(ring);
}
