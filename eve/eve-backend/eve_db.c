#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
//#include <stdint.h>
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
//#include <cbor.h>
#include <cjson/cJSON.h>
#include <eve.h>

#define ERROR(...) do { fprintf(stderr, __VA_ARGS__); exit(1); } while (0)
#define MSG_OUT stdout /* Send info to stdout, change to stderr if you want */
#define DPRINT(x...) printf(x)

#define BUFFER_SIZE 1024

#define BUFF_SZ   512
char buff[BUFF_SZ + 1];

#define EVE_DEVEL 1
#define EVE_MAGIC  0xE4E4E4E4
#define EVE_DATA_VERSION   ((EVE_DEVEL) ? 999 : 1)
//#define EVE_DATA_VERSION 1

struct _eve_db_header {
  uint32_t magic;
  uint8_t version;
  // zero or one
  uint8_t active_database;
  time_t first_minute;
  time_t next_roll_over_minute;
};
typedef struct _eve_db_header eve_db_header_t;

// Timeframes:
// realtime
// 1s
// 5s
// 1m
// 5m

// .2s
// File Format:
//   Bytes 0-4095: Header
//   Bytes 4096 - 86015: Database A
//   Bytes 86016 - 167935: Database B
//   Bytes 167936+: CBOR data
#define DB_A_OFFSET 4096
#define DB_B_OFFSET 86016
#define CBOR_OFFSET 167936

struct _eve_point_two_second_header {
  uint64_t header_array[10080];
};
typedef struct _eve_point_two_second_header eve_point_two_second_header_t;

enum object_type {
  tcp_stream_stats = 1
};

struct tcp_stream_stats {
  enum object_type object_type;
  uint32_t egress_window_size;
  uint32_t ingress_window_size;
  uint64_t ingress_bytes;
  uint64_t egress_bytes;
};

#define BUFF_SZ   512

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

int main(int argc, char **argv) {
  struct ev_loop *loop = ev_default_loop(0);

  eve_db_header_t *header = malloc(sizeof(eve_db_header_t));
  header->magic = htonl(EVE_MAGIC);
  //header->version = htonl(EVE_DATA_VERSION);
  header->version = EVE_DATA_VERSION;
  header->active_database = 0;
  header->first_minute = time(NULL) / 60 * 60;
  header->next_roll_over_minute = header->first_minute + 10080;

  printf("Now, as seconds, rounded down to the nearest full minute is %ju\n", (uintmax_t)header->first_minute);
  printf("The time to close the file, in seconds, is %ju\n", (uintmax_t)header->next_roll_over_minute);
  int fd;

  // Open a file in writing mode
  fd = open("12345678.eve", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd == -1) {
    perror("Error opening file");
    return 1;
  }

  ssize_t bytes_written = write(fd, header, sizeof(eve_db_header_t));

  if (bytes_written == -1) {
    perror("Error writing to file");
    close(fd);  // Close the file before returning on error
    exit(1);
  }

  
  eve_point_two_second_header_t *db_header = malloc(sizeof(eve_point_two_second_header_t));

  bytes_written = pwrite(fd, db_header, sizeof(eve_point_two_second_header_t), DB_A_OFFSET);
  if (bytes_written == -1) {
    perror("Error writing to file");
    close(fd);  // Close the file before returning on error
    exit(1);
  }

  bytes_written = pwrite(fd, db_header, sizeof(eve_point_two_second_header_t), DB_B_OFFSET);
  if (bytes_written == -1) {
    perror("Error writing to file");
    close(fd);  // Close the file before returning on error
    exit(1);
  }

  // Close the file
  if (close(fd) == -1) {
    perror("Error closing file");
    exit(1);
  }

  printf("Data written to the file successfully.\n");

  //ssize_t written = write(fptr, const void *buf, size_t count);

  //ev_timer timeout_watcher;
  //ev_timer_init (&timeout_watcher, timeout_cb, 0.5, 10.);
  //ev_timer_start (loop, &timeout_watcher);

  //eve_iou_init(loop);

  //ev_run (loop, 0);

  //abort();

  //printf("Connection closed\n");
}
