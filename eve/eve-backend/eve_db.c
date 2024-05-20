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
#include <cbor.h>
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
  uint64_t first_minute;
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

struct _eve_point_two_second_header {
  uint64_t header_array[10080];
};
typedef struct _eve_point_two_second_header eve_point_two_second_header_t;

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

void hexDump(char *desc, void *addr, int len) {
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

static void timeout_cb (EV_P_ ev_timer *w, int revents) {
  fprintf (stderr, "timeout\n");
}

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
  header->first_minute = 1;

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
