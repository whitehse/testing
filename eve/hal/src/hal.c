/* Standard Headers */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

/* mmap support */
#include <sys/mman.h>

/* Socket/Network Headers  */
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <linux/if_packet.h>
//#include <linux/if_ether.h>
#include <linux/filter.h>
struct ucred {
  __u32   pid;
  __u32   uid;
  __u32   gid;
};

/* netlink and cfg80211 headers */
#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
//#include <netlink/msg.h>
#include <linux/nl80211.h>
#include <sys/random.h>

//#include <hal_config.h>

/* openssl non-blocking helper */
/* https://github.com/darrenjs/openssl_examples */
//*#include <darrenjs/common.h>
//#include <monocypher.h>
//#include <sodium.h>

//#include "ev.h"
#define EV_STANDALONE 1
#include "ev.c"

#include <cbor.h>
#include <cjson/cJSON.h>
#include <termios.h>
 
/* https://github.com/DavidLeeds/hashmap */
#include <hashmap.h>

#define RING_FRAMES     256
#define PKT_OFFSET      (TPACKET_ALIGN(sizeof(struct tpacket2_hdr)) + \
                         TPACKET_ALIGN(sizeof(struct sockaddr_ll))) + 2

static struct {
    struct nl_sock *nls;
    int nl80211_id;
} wifi;

struct ev_loop *loop;
ev_timer timeout_watcher;
int dl_len;
int32_t total_frames_captured = 0;
int page_size;
int block_size;
int frame_size;
int bad_frames;

struct listen_data {
    char* ring;
    int tp_block_size;
    int tp_block_nr;
    int tp_frame_size;
    int tp_frame_nr;
    int offset;
};

struct ring {
	struct iovec *rd;
	uint8_t *map;
	struct tpacket_req req;
};

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

static void stdin_cb (EV_P_ ev_io *w, int revents) {
  char buffer[1024];
  ssize_t r;
  if(EV_ERROR & revents) {
    perror("got invalid event");
    return;
  }

  // Read a line from stdin
  r = read(STDIN_FILENO, buffer, 1024);

  if (r = -1) {
    puts("r = -1");
    return 0;
  }

  if (r == 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      perror("Something weird happened");
      return 0;
    } else {
      perror("Something bad happened");
      return 1;
    }
  }

  struct cbor_load_result result;
  printf("Read %d bytes from stdin.\n", r);
  cbor_item_t* item = cbor_load(buffer, r, &result);
  //free(buffer);

  if (result.error.code != CBOR_ERR_NONE) {
    printf(
        "SERVER: There was an error while reading the input near byte %zu (read %zu "
        "bytes in total): ",
        result.error.position, result.read);
    switch (result.error.code) {
      case CBOR_ERR_MALFORMATED: {
        printf("SERVER: Malformed data\n");
        break;
      }
      case CBOR_ERR_MEMERROR: {
        printf("SERVER: Memory error -- perhaps the input is too large?\n");
        break;
      }
      case CBOR_ERR_NODATA: {
        printf("SERVER: The input is empty\n");
        break;
      }
      case CBOR_ERR_NOTENOUGHDATA: {
        printf("SERVER: Data seem to be missing -- is the input complete?\n");
        break;
      }
      case CBOR_ERR_SYNTAXERROR: {
        printf(
            "SERVER: Syntactically malformed data -- see "
            "https://www.rfc-editor.org/info/std94\n");
        break;
      }
      case CBOR_ERR_NONE: {
        // GCC's cheap dataflow analysis gag
        break;
      }
    }
  } else {
    printf("SERVER: CBOR data was properly formatted\n");

    cbor_describe(item, stdout);

    cJSON* cjson_item = cbor_to_cjson(item);
    char* json_string = cJSON_Print(cjson_item);
    printf("SERVER: %s\n", json_string);
    //free(json_string);
  }
}

static void timeout_cb (EV_P_ ev_timer *w, int revents) {
  //printf("timeout. Total frames captured: %d, bad frames %d\n", total_frames_captured, bad_frames);
  cbor_item_t* root = cbor_new_definite_map(2);
  /* Add the content */
  bool success = cbor_map_add(
      root, (struct cbor_pair){
                .key = cbor_move(cbor_build_string("Is CBOR awesome?")),
                .value = cbor_move(cbor_build_bool(true))});
  success &= cbor_map_add(
      root, (struct cbor_pair){
                .key = cbor_move(cbor_build_uint8(42)),
                .value = cbor_move(cbor_build_string("Is the answer"))});
  if (!success) return 1;
  /* Output: `length` bytes of data in the `buffer` */
  unsigned char* cbuffer;
  size_t cbuffer_size;
  cbor_serialize_alloc(root, &cbuffer, &cbuffer_size);
  uint8_t cbuffer_len = (uint8_t)cbuffer_size;

  fwrite(cbuffer, 1, cbuffer_size, stdout);

}

// Ethernet data structure, to be filled out by the ethernet
// sub module and further populated downstream.
//
// It's inialized to NULL in main, and then populated the
// first time it gets passed to ethernet
char *ethernet_ptr;

static void wire_cb (EV_P_ ev_io *w, int revents) {
  struct tpacket2_hdr *header;
  struct listen_data *listen_struct = w->data;

/*
  listen_struct->tp_block_size = tp.tp_block_size;
  listen_struct->tp_block_nr = tp.tp_block_nr;
  listen_struct->tp_frame_size = tp.tp_frame_size;
  listen_struct->tp_frame_nr = tp.tp_frame_nr;
*/

  char *ring = listen_struct->ring;
  int offset = listen_struct->offset;

  header = (void *) ring + (offset * frame_size);

  while (header->tp_status & TP_STATUS_USER) {
    total_frames_captured += 1;

    int ret = hal_eth_parse(&ethernet_ptr, ((void *) header) + PKT_OFFSET, header->tp_len);
    if (ret) {
      bad_frames += 1;
    }   
    header->tp_status = 0;

    offset = (offset + 1) % RING_FRAMES;
    header = (void *) ring + (offset * frame_size);
    //offset = (offset + 1) % listen_struct->tp_frame_nr;

    //int buffer_idx = offset / (listen_struct.tp_block_size / listen_struct.tp_frame_size);
    //char* buffer_ptr = ring + buffer_idx * listen_struct.tp_block_size;

    // Determine the location of the frame within that buffer.
    //int frame_idx_diff = frame_idx % frames_per_buffer;
    //frame_ptr = buffer_ptr + frame_idx_diff * req.tp_frame_size;
  }

  listen_struct->offset = offset;
}

void listen_on_interface (char* interface) {
  struct tpacket_req tp;
  struct sockaddr_ll my_addr;
  struct ifreq s_ifr;
  ev_io *wire_watcher;
  int wire_fd;
  char* ring;
  struct packet_mreq mreq = {0};
  struct listen_data *listen_struct;
  listen_struct = malloc(sizeof(struct listen_data));

  tp.tp_block_size = RING_FRAMES * getpagesize();
  tp.tp_block_nr = 1;
  tp.tp_frame_size = getpagesize();
  tp.tp_frame_nr = RING_FRAMES;

  wire_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (wire_fd < 0) {
    perror("Couldn't create socket");
    puts("Couldn't create socket");
    abort();
  } else {
    puts("Created the socket");
  }
  //perror("Called socket");

  int version = TPACKET_V2;
  if ((setsockopt(wire_fd, SOL_PACKET, PACKET_VERSION, &version, sizeof(version))) == -1) {
    perror("setsockopt");
    abort();
  }
  //perror("setsockopt, set to TPACKET_V2");

  struct tpacket2_hdr *header;
  if (setsockopt(wire_fd, SOL_PACKET, PACKET_RX_RING, (void*) &tp, sizeof(tp)) < 0) {
      perror("Couldn't set PACKET_RX_RING");
      abort();
  }
  //perror("setsockopt for packet_rx_ring");

  strncpy (s_ifr.ifr_name, interface, sizeof(s_ifr.ifr_name));

  // Retrive interface index
  if(ioctl(wire_fd, SIOCGIFINDEX, &s_ifr)==-1) {
        perror("Finding interface index failed");
        abort();
  }
  //perror("called ioctl to get network index");

  mreq.mr_ifindex = if_nametoindex(interface);
  mreq.mr_type = PACKET_MR_PROMISC;

  // Listen Promiscuously
  if (setsockopt(wire_fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) != 0) {
    perror("unable to enter promiscouous mode");
    abort();
  }
  //perror("called setsockopt to set promiscuous mode");
  //printf("set '%s' to promisc succeeded!\n",interface);

  my_addr.sll_family = AF_PACKET;
  my_addr.sll_protocol = htons(ETH_P_ALL);
  my_addr.sll_ifindex =  s_ifr.ifr_ifindex;

  bind(wire_fd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr_ll));

  ring = mmap(0, tp.tp_block_size * tp.tp_block_nr,
              PROT_READ | PROT_WRITE, MAP_SHARED, wire_fd, 0);

  if (!ring) {
      puts("The call to mmap did not return data for ring");
      abort();
  }

  listen_struct->ring = ring;
  listen_struct->offset = 0;
  listen_struct->tp_block_size = tp.tp_block_size;
  listen_struct->tp_block_nr = tp.tp_block_nr;
  listen_struct->tp_frame_size = tp.tp_frame_size;
  listen_struct->tp_frame_nr = tp.tp_frame_nr;

  wire_watcher = malloc(sizeof(struct ev_io));
  ev_io_init(wire_watcher, wire_cb, wire_fd, EV_READ);
  wire_watcher->data = listen_struct;
  ev_io_start(loop, wire_watcher);
}

static int list_interface_handler(struct nl_msg *msg, void *arg) {
  struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];
  struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));

  nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
            genlmsg_attrlen(gnlh, 0), NULL);

  if (tb_msg[NL80211_ATTR_IFNAME]) {
    //printf("Interface: %s\n", nla_get_string(tb_msg[NL80211_ATTR_IFNAME]));
  }
  return NL_SKIP;
}

static int finish_handler(struct nl_msg *msg, void *arg) {
  int *ret = arg;
  *ret = 0;
  return NL_SKIP;
}

int main(void) {
  loop = EV_DEFAULT;

  setvbuf(stdout, NULL, _IONBF, 0);

  struct termios stored_settings;
  tcgetattr(0, &stored_settings);

  // copy existing setting flags
  struct termios new_settings = stored_settings;

  // modify flags
  // first, disable canonical mode
  // (canonical mode is the typical line-oriented input method)
  new_settings.c_lflag &= (~ICANON);
  new_settings.c_lflag &= (~ECHO); // don't echo the character
  //new_settings.c_lflag &= (~ISIG); // don't automatically handle control-C
  //new_settings.c_cc[VTIME] = 1; // timeout (tenths of a second)
  new_settings.c_cc[VTIME] = 1; // timeout (tenths of a second)
  new_settings.c_cc[VMIN] = 0; // minimum number of characters

  // apply the new settings
  tcsetattr(0, TCSANOW, &new_settings);

  //setvbuf(0, NULL, _IONBF, 0);
  int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

  total_frames_captured = 0;
  bad_frames = 0;
  ethernet_ptr = NULL;

  frame_size = getpagesize();

  block_size = frame_size * RING_FRAMES;
  
  //listen_on_interface("wlp4s0");
  //listen_on_interface("wlp146s0");
  //listen_on_interface("wlo1");
  //listen_on_interface("ath0");
  //listen_on_interface("ath01");
  //listen_on_interface("ath08");
  //listen_on_interface("wlp0s12f0");
  listen_on_interface("br-lan");

  wifi.nls = nl_socket_alloc();
  if (!wifi.nls) {
    fprintf(stderr, "Failed to allocate netlink socket.\n");
    return -ENOMEM;
  }
  
  // TODO I probably don't want to use getpagesize() here
  nl_socket_set_buffer_size(wifi.nls, getpagesize(), getpagesize());

  if (genl_connect(wifi.nls)) {
    fprintf(stderr, "Failed to connect to generic netlink.\n");
    nl_socket_free(wifi.nls);
    return -ENOLINK;
  }

  wifi.nl80211_id = genl_ctrl_resolve(wifi.nls, "nl80211");
  if (wifi.nl80211_id < 0) {
    fprintf(stderr, "nl80211 not found.\n");
    nl_socket_free(wifi.nls);
    return -ENOENT;
  }

  struct nl_msg *msg = nlmsg_alloc();
  if (!msg) {
    fprintf(stderr, "Failed to allocate netlink message.\n");
    return -ENOMEM;
  }

  struct nl_cb *cb = nl_cb_alloc(NL_CB_DEFAULT);
  if (!cb) {
    fprintf(stderr, "Failed to allocate netlink callback.\n");
    nlmsg_free(msg);
    return -ENOMEM;
  }

  nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, list_interface_handler, NULL);

  genlmsg_put(msg, 0, 0, wifi.nl80211_id, 0,
              NLM_F_DUMP, NL80211_CMD_GET_INTERFACE, 0);

  nl_send_auto_complete(wifi.nls, msg);

  int err = 1;

  nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &err);

  while (err > 0) {
    nl_recvmsgs(wifi.nls, cb);
  }

  nlmsg_free(msg);
  nl_cb_put(cb);
  nl_socket_free(wifi.nls);

  ev_io *stdin_watcher;
  stdin_watcher = malloc(sizeof(ev_io));
  ev_io_init (stdin_watcher, stdin_cb, /*STDIN_FILENO*/ 0, EV_READ);
  ev_io_start (loop, stdin_watcher);

  ev_timer_init (&timeout_watcher, timeout_cb, 1., 10.);
  ev_timer_start (loop, &timeout_watcher);

  //printf("Hello world!\n");
  ev_run (loop, 0);
  printf("What happened?\n");
  return 0;
}
