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

//#include <hal_config.h>

/* openssl non-blocking helper */
/* https://github.com/darrenjs/openssl_examples */
//*#include <darrenjs/common.h>
//#include <sodium.h>
#include <monocypher.h>
#include <sys/random.h>

/* libev header */
//#include "ev.h"
#define EV_STANDALONE 1
#include "ev.c"
 
/* https://github.com/DavidLeeds/hashmap */
#include <hashmap.h>

/* Custom protobuf headers */
//#include <axos.pb-c.h>
//#include <objects.pb-c.h>

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

static void timeout_cb (EV_P_ ev_timer *w, int revents) {
  //printf("timeout. Total frames captured: %d, bad frames %d\n", total_frames_captured, bad_frames);
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
    abort();
  }
  perror("Called socket");

  int version = TPACKET_V2;
  if ((setsockopt(wire_fd, SOL_PACKET, PACKET_VERSION, &version, sizeof(version))) == -1) {
    perror("setsockopt");
    abort();
  }
  perror("setsockopt, set to TPACKET_V2");

  struct tpacket2_hdr *header;
  if (setsockopt(wire_fd, SOL_PACKET, PACKET_RX_RING, (void*) &tp, sizeof(tp)) < 0) {
      perror("Couldn't set PACKET_RX_RING");
      abort();
  }
  perror("setsockopt for packet_rx_ring");

  strncpy (s_ifr.ifr_name, interface, sizeof(s_ifr.ifr_name));

  // Retrive interface index
  if(ioctl(wire_fd, SIOCGIFINDEX, &s_ifr)==-1) {
        perror("Finding interface index failed");
        abort();
  }
  perror("called ioctl to get network index");

  mreq.mr_ifindex = if_nametoindex(interface);
  mreq.mr_type = PACKET_MR_PROMISC;

  // Listen Promiscuously
  if (setsockopt(wire_fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) != 0) {
    perror("unable to enter promiscouous mode");
    abort();
  }
  perror("called setsockopt to set promiscuous mode");
  printf("set '%s' to promisc succeeded!\n",interface);

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
    printf("Interface: %s\n", nla_get_string(tb_msg[NL80211_ATTR_IFNAME]));
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

  //printf("Project Name: %s\n", project_name);
  //printf("Project Version: %s\n", project_version);

  total_frames_captured = 0;
  bad_frames = 0;
  ethernet_ptr = NULL;

  frame_size = getpagesize();

  block_size = frame_size * RING_FRAMES;
  
  //listen_on_interface("wlp4s0");
  //listen_on_interface("wlp0s12f0");
  listen_on_interface("wlp146s0");
  //listen_on_interface("wlo1");
  //listen_on_interface("ath0");
  //listen_on_interface("ath01");
  //listen_on_interface("ath08");
  //listen_on_interface("br-lan");

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

/*
  int tlsfd = socket(AF_INET, SOCK_STREAM, 0);
  if (tlsfd < 0) {
    perror("tlssocket");
    abort();
  }

  char* host_ip = "127.0.0.1";
  struct sockaddr_in tlsaddr;
  memset(&tlsaddr, 0, sizeof(tlsaddr));
  tlsaddr.sin_family = AF_INET;
  tlsaddr.sin_port = htons(55555);

  if (inet_pton(AF_INET, host_ip, &(tlsaddr.sin_addr)) <= 0) {
    die("inet_pton()");
  }

  if (connect(tlsfd, (struct sockaddr*) &tlsaddr, sizeof(tlsaddr)) < 0) {
    die("connect()");
  }
  printf("tls socket connected\n");
  ssl_init(0,0);
*/

  uint8_t key        [32];    /* Random, secret session key  */
  uint8_t nonce      [24];    /* Use only once per key       */
  uint8_t plain_text [12] = "Lorem ipsum"; /* Secret message */
  uint8_t mac        [16];    /* Message authentication code */
  uint8_t cipher_text[12];              /* Encrypted message */
  getrandom(key, 32, 0);
  getrandom(nonce, 24, 0);

  crypto_aead_lock(cipher_text, mac,
    key, nonce,
    NULL, 0,
    plain_text, sizeof(plain_text));

  /* Wipe secrets if they are no longer needed */
  crypto_wipe(plain_text, 12);
  crypto_wipe(key, 32);

  ev_timer_init (&timeout_watcher, timeout_cb, 5.5, 10);
  ev_timer_start (loop, &timeout_watcher);

  printf("Hello world!\n");
  ev_run (loop, 0);
  printf("What happened?\n");
  return 0;
}
