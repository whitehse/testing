#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <errno.h>
//#include <hal/ethernet.h>
#include <hal/ipv4.h>
#include <hal/tcp.h>
#include <hashmap.h>
#include <ev.h>

struct tcp_socket {
    uint32_t peer_a_ip;
    uint16_t peer_a_port;
    uint32_t peer_b_ip;
    uint16_t peer_b_port;
};

int v4tcpsocketcmp (uint32_t source_ip, uint16_t source_port, uint32_t destination_ip, uint16_t destination_port) {
    int r=0;
    if (source_ip == destination_ip) {
        if (source_port < destination_port) {
            r = -1;
        } else if (source_port > destination_port) {
            r = 1;
        } else {
            r = 0;
	}
    } else if (source_ip < destination_ip) {
        r = -1;
    } else {
        r = 1;
    }
    //printf("source ip=%zu, destination ip=%zu, result=%d\n", source_ip, destination_ip, r);
    return r;
}

int compare_tcp_sockets(struct tcp_socket *socket_a, struct tcp_socket *socket_b){
    if (socket_a->peer_a_ip == socket_b->peer_a_ip)
        if (socket_a->peer_b_ip == socket_b->peer_b_ip)
            if (socket_a->peer_a_port == socket_b->peer_a_port)
                if (socket_a->peer_b_port == socket_b->peer_b_port)
		    return 0;
		else if (socket_a->peer_b_port > socket_b->peer_b_port)
		    return 1;
		else
                    return -1;
            else if (socket_a->peer_a_port > socket_b->peer_a_port)
                return 1;
            else
                return -1;
        else if (socket_a->peer_b_ip > socket_b->peer_b_ip)
            return 1;
	else
            return -1;
    else if (socket_a->peer_a_ip > socket_b->peer_a_ip)
        return 1;
    else
        return -1;
}

//hashmap_set_key_alloc_funcs(&tcp_map, dup_tcp_socket, free_tcp_socket);

struct tcp_socket* dup_tcp_socket(struct tcp_socket *socket) {
    //puts("dup_tcp_socket called");
    struct tcp_socket *new_socket = malloc(sizeof(struct tcp_socket));
    memcpy(new_socket, socket, sizeof(struct tcp_socket));
    return new_socket;
}

free_tcp_socket(struct tcp_socket *socket) {
    //puts("free_tcp_socket called");
    free(socket);
}

#define HAL_TCP_STATE_SYN_SENT       1
#define HAL_TCP_STATE_SYN_RECEIVED   2
#define HAL_TCP_STATE_ESTABLISHED    3
#define HAL_TCP_STATE_FIN_WAIT_1     4
#define HAL_TCP_STATE_FIN_WAIT_2     4
#define HAL_TCP_STATE_CLOSE_WAIT     5
#define HAL_TCP_STATE_CLOSING        6
#define HAL_TCP_STATE_LAST_ACK       7
#define HAL_TCP_STATE_TIME_WAIT      8
#define HAL_TCP_STATE_CLOSED         9

struct tcp_connection {
    uint8_t state;
    uint8_t peer_a_offered_window_scaling;
    uint8_t peer_b_offered_window_scaling;
    uint8_t peer_a_window_scale;
    uint8_t peer_b_window_scale;
    uint8_t window_scaling_enabled;
    struct tcp_socket *socket;
    uint64_t peer_a_bytes;
    uint64_t peer_b_bytes;
    ev_timer timeout_watcher;
};

size_t hashmap_tcp_socket(struct tcp_socket *socket) {
    size_t r = hashmap_hash_default(socket, sizeof(struct tcp_socket));
    //printf("hash=%u. peer a ip=%zu, peer a port=%u, peer b ip=%zu, peer b port=%u\n", r, socket->peer_a_ip, socket->peer_b_ip, socket->peer_a_port, socket->peer_b_port);
    return r;
}

typedef HASHMAP(struct tcp_socket, struct tcp_connection) tcp_hash_t;
tcp_hash_t tcp_map;

static void syn_cb (EV_P_ ev_timer *w, int revents) {
  struct tcp_connection *connection = w->data;
  struct tcp_socket *socket = connection->socket;
  puts("====syn_cb called===");
  ev_timer_stop(loop, &(connection->timeout_watcher));
  struct tcp_connection *val = hashmap_remove(&tcp_map, socket);
  if (val == NULL) {
    puts("val is NULL when attempting to remove tcp_map entry");
    abort();
  }
  //free(val);
  //puts("Removed and deleted hashmap entry.");
  free(socket);
  free(connection);
}

int tcp_parser(struct hal_ipv4 *ipv4) {
    //static tcp_hash_t tcp_map;
    static struct hal_tcp tcp;
    static tcp_init = 0;
    struct ev_loop *loop = EV_DEFAULT;
    static uint64_t total_tcp_bytes = 0;
    static int bad_segments = 0;
    static uint64_t connections_not_found=0;
    static uint64_t connections_found=0;

    if (tcp_init == 0) {
        printf("tcp is null\n");
        hashmap_init(&tcp_map, hashmap_tcp_socket, compare_tcp_sockets);
	//hashmap_set_key_alloc_funcs(&tcp_map, dup_tcp_socket, free_tcp_socket);
	//#define hashmap_set_key_alloc_funcs(h, key_dup_func, key_free_func) do {
	tcp_init = 1;
    }

    memset(&tcp, 0, sizeof(struct hal_tcp));

    if (ipv4->protocol != HAL_IPV4_PROT_TCP) {
	return HAL_TCP_NA;
    }

    char* buf = ipv4->payload_pointer;

    if (ipv4->payload_length < HAL_TCP_MIN_SEGMENT_SIZE || ipv4->payload_length > HAL_TCP_MAX_SEGMENT_SIZE) {
        printf("TCP segment had the wrong size\n");
        bad_segments += 1;
        return HAL_TCP_BADPARAM;
    }

    tcp.source_port = ntohs(*(uint16_t*)buf);
    tcp.destination_port = ntohs(*(uint16_t*)(buf+2));
    tcp.seq_number = ntohl(*(uint32_t*)(buf+4));
    tcp.ack_number = ntohl(*(uint32_t*)(buf+8));
    uint16_t offset_and_flags = ntohs(*(uint16_t*)(buf+12));
    tcp.offset = offset_and_flags >> 12;
    tcp.flags = offset_and_flags & 0x1ff;
    tcp.window_size = ntohs(*(uint16_t*)(buf+14));
    tcp.checksum = ntohs(*(uint16_t*)(buf+16));
    tcp.urgent_pointer = ntohs(*(uint16_t*)(buf+18));
    tcp.payload_length = ipv4->payload_length - tcp.offset*4;
    total_tcp_bytes += tcp.payload_length;

    if (tcp.source_port == 30007 || tcp.destination_port == 30007) {
        return HAL_TCP_OK;
    }

    int counter = 0;
    // parse options, if any
    int option_len = tcp.offset*4 - 20;
    if (option_len > 0) {
      char* option_pointer = buf + 20;
      //int counter = 0;
      while (option_pointer[0] != 0 && counter < option_len) {
	switch(option_pointer[0]) {
	  // noop
          case 1:
            counter += 1;
	    option_pointer += 1;
	    break;
	  // maximum segment size
	  case 2:
	    if (counter + 4 > option_len) { // Don't read beyond the header into the payload
              printf("Switch case 2. Option overlaps payload. option_len=%d\n", option_len);
              bad_segments += 1;
              return HAL_TCP_BADPARAM;
	    }
	    tcp.maximum_segment_size_exists = 1;
            tcp.maximum_segment_size = ntohs(*(uint16_t*)(option_pointer+2));
            counter += 4;
	    option_pointer += 4;
	    break;
	  // window scale
	  case 3:
	    if (counter + 3 > option_len) {
              printf("Switch case 3. Option overlaps payload.\n");
              bad_segments += 1;
              return HAL_TCP_BADPARAM;
	    }
	    tcp.window_scale_exists = 1;
            tcp.window_scale = (uint8_t)option_pointer[2];
	    //printf("window scale=%u.\n", tcp.window_scale);
            counter += 3;
	    option_pointer += 3;
	    break;
	  // selective acknowledgement permitted
	  case 4:
	    if (counter + 2 > option_len) {
              printf("Switch case 4. Option overlaps payload.\n");
              bad_segments += 1;
              return HAL_TCP_BADPARAM;
	    }
	    tcp.sack_allowed_exists = 1;
            counter += 2;
	    option_pointer += 2;
	    break;
	  // selective acknowledgement
	  case 5:
	    if (counter + 2 > option_len) {
              printf("Switch case 5. Option overlaps payload.\n");
              bad_segments += 1;
              return HAL_TCP_BADPARAM;
	    }
	    uint8_t sack_len = (option_pointer+1)[0];
	    if (counter + sack_len > option_len) {
              printf("Switch case 5. Option overlaps payload. sack_len=%d\n", sack_len);
              bad_segments += 1;
	      return HAL_TCP_BADPARAM;
	    }
	    tcp.sack_block_count = (sack_len - 2) / 8;
	    switch(tcp.sack_block_count) {
              case 1:
		tcp.sack_block_1 = ntohs(*(uint16_t*)(option_pointer+2));
                break;
              case 2:
		tcp.sack_block_1 = ntohs(*(uint16_t*)(option_pointer+2));
		tcp.sack_block_2 = ntohs(*(uint16_t*)(option_pointer+10));
                break;
              case 3:
		tcp.sack_block_1 = ntohs(*(uint16_t*)(option_pointer+2));
		tcp.sack_block_2 = ntohs(*(uint16_t*)(option_pointer+10));
		tcp.sack_block_2 = ntohs(*(uint16_t*)(option_pointer+18));
                break;
              case 4:
		tcp.sack_block_1 = ntohs(*(uint16_t*)(option_pointer+2));
		tcp.sack_block_2 = ntohs(*(uint16_t*)(option_pointer+10));
		tcp.sack_block_3 = ntohs(*(uint16_t*)(option_pointer+18));
		tcp.sack_block_4 = ntohs(*(uint16_t*)(option_pointer+26));
                break;
              default:
		printf("Switch case 5. The sack options length is bad\n");
                bad_segments += 1;
	        return HAL_TCP_BADPARAM;
	    }
            counter += 2 + sack_len;
	    option_pointer += 2 + sack_len;
	    break;
	  // timestamp
	  case 8:
	    if (counter + 10 > option_len) {
              printf("Switch case 8. Option overlaps payload.\n");
              bad_segments += 1;
              return HAL_TCP_BADPARAM;
	    }
	    tcp.timestamp_exists = 1;
            tcp.timestamp_value = ntohs(*(uint16_t*)(option_pointer+2));
            tcp.timestamp_echo_reply = ntohs(*(uint16_t*)(option_pointer+6));
            counter += 10;
	    option_pointer += 10;
	    break;
	  // ignore unknown options, All options except for 0 and 1 must
	  // include length fields
	  default:
	    if (counter + 2 > option_len) {
              printf("Switch case Other. Option overlaps payload.\n");
              bad_segments += 1;
              return HAL_TCP_BADPARAM;
	    }
	    uint8_t len = (option_pointer+1)[0];
	    if (counter + 2 + len > option_len) {
              printf("Switch case Other. Option overlaps payload.\n");
              bad_segments += 1;
	      return HAL_TCP_BADPARAM;
	    }
            counter += 2 + len;
	    option_pointer += 2 + len;
	    break;
	}
      }
    }

/*
    printf("tcp:source port=%d, destination_port=%d, seq number=%d, ack number=%d, window size=%d, header size=%d bytes payload size=%d, total=%lld, flags: %s%s%s%s%s%s%s%s%s, option bytes=%d, timestamp=%d, timestamp_echo=%d, bad segments=%d\n",
        tcp.source_port,
	tcp.destination_port,
	tcp.seq_number,
	tcp.ack_number,
	tcp.window_size,
	tcp.offset*4,
	tcp.payload_length,
	total_tcp_bytes,
	tcp.flags & HAL_TCP_FLAG_FIN ? "F" : "",
	tcp.flags & HAL_TCP_FLAG_SYN ? "S" : "",
	tcp.flags & HAL_TCP_FLAG_RST ? "R" : "",
	tcp.flags & HAL_TCP_FLAG_PSH ? "P" : "",
	tcp.flags & HAL_TCP_FLAG_ACK ? "A" : "",
	tcp.flags & HAL_TCP_FLAG_URG ? "U" : "",
	tcp.flags & HAL_TCP_FLAG_ECE ? "E" : "",
	tcp.flags & HAL_TCP_FLAG_CWR ? "C" : "",
	tcp.flags & HAL_TCP_FLAG_NS  ? "N" : "",
	counter,
	tcp.timestamp_value,
	tcp.timestamp_echo_reply,
	bad_segments
    );
*/

    if (ipv4->payload_length < tcp.offset) {
        printf("TCP segment had the wrong size\n");
        bad_segments += 1;
        return HAL_TCP_BADPARAM;
    }

    // If this is an existing connection, update the byte counters
    struct tcp_socket socket;
    memset(&socket, 0, sizeof(struct tcp_socket));
    int source_is_greater = v4tcpsocketcmp(ipv4->source_ip, tcp.source_port, ipv4->destination_ip, tcp.destination_port);
    if (source_is_greater > 0) {
      socket.peer_b_ip = ipv4->source_ip;
      socket.peer_b_port = tcp.source_port;
      socket.peer_a_ip = ipv4->destination_ip;
      socket.peer_a_port = tcp.destination_port;
    } else {
      socket.peer_a_ip = ipv4->source_ip;
      socket.peer_a_port = tcp.source_port;
      socket.peer_b_ip = ipv4->destination_ip;
      socket.peer_b_port = tcp.destination_port;
    }
    struct tcp_connection *connection = hashmap_get(&tcp_map, &socket);
    if (connection == NULL) {
      // There is no existing connection
      connections_not_found += 1;
    } else {
      connections_found += 1;
      if (source_is_greater > 0) {
        connection->peer_b_bytes += tcp.payload_length;
      } else {
        connection->peer_a_bytes += tcp.payload_length;
      }
    }

    if(
      (tcp.flags & HAL_TCP_FLAG_SYN) &&
      (tcp.flags & HAL_TCP_FLAG_ACK) == 0) {
        //printf("SYN\n");
        struct tcp_socket *socket = malloc(sizeof(struct tcp_socket));
	memset(socket, 0, sizeof(struct tcp_socket));
        struct tcp_connection *connection = malloc(sizeof(struct tcp_connection));
	memset(connection, 0, sizeof(struct tcp_connection));
        if (v4tcpsocketcmp(ipv4->source_ip, tcp.source_port, ipv4->destination_ip, tcp.destination_port) > 0) {
	  socket->peer_b_ip = ipv4->source_ip;
	  socket->peer_b_port = tcp.source_port;
	  socket->peer_a_ip = ipv4->destination_ip;
	  socket->peer_a_port = tcp.destination_port;
	  if (tcp.window_scale_exists) {
            connection->peer_b_window_scale = tcp.window_scale;
            connection->peer_b_offered_window_scaling = 1;
	  }
	} else {
	  socket->peer_a_ip = ipv4->source_ip;
	  socket->peer_a_port = tcp.source_port;
	  socket->peer_b_ip = ipv4->destination_ip;
	  socket->peer_b_port = tcp.destination_port;
	  if (tcp.window_scale_exists) {
            connection->peer_a_window_scale = tcp.window_scale;
            connection->peer_a_offered_window_scaling = 1;
	  }
	}
	connection->state = HAL_TCP_STATE_SYN_SENT;
	connection->socket = socket;

  	if (hashmap_put(&tcp_map, socket, connection) == -EEXIST) {
	  //puts("socket alreadys exists");
	  free(connection);
  	  struct tcp_connection *connection = hashmap_get(&tcp_map, socket);
	  if (connection == NULL) {
	    puts("Did not find existing connection when it just told us that it existed.");
	    free(socket);
	    abort();
	  }
	  free(socket);
	  ev_timer_set(&(connection->timeout_watcher), 75, 0.);
	} else {
	  //puts("inserted socket");
	  connection->timeout_watcher.data = connection;
          ev_timer_init (&(connection->timeout_watcher), syn_cb, 75, 0.);
          ev_timer_start (loop, &(connection->timeout_watcher));
	}
	
	/*
	int i=0;
	struct tcp_socket *key;
	hashmap_foreach_key(key, &tcp_map) {
	  i++;
        }
	HASHMAP_ITER(tcp_map) it;
	for (it = hashmap_iter(&tcp_map); hashmap_iter_valid(&it); hashmap_iter_next(&it)) {
	  i++;
	}
	printf("There are %d entries in the hashtable.\n", i);
	*/
    }
    if(
      (tcp.flags & HAL_TCP_FLAG_SYN) &&
      (tcp.flags & HAL_TCP_FLAG_ACK)) {
        //printf("SYN ACK\n");
        struct tcp_socket socket;
	memset(&socket, 0, sizeof(struct tcp_socket));
	int source_is_greater = v4tcpsocketcmp(ipv4->source_ip, tcp.source_port, ipv4->destination_ip, tcp.destination_port);
	if (source_is_greater > 0) {
	  socket.peer_b_ip = ipv4->source_ip;
	  socket.peer_b_port = tcp.source_port;
	  socket.peer_a_ip = ipv4->destination_ip;
	  socket.peer_a_port = tcp.destination_port;
	} else {
	  socket.peer_a_ip = ipv4->source_ip;
	  socket.peer_a_port = tcp.source_port;
	  socket.peer_b_ip = ipv4->destination_ip;
	  socket.peer_b_port = tcp.destination_port;
	}
	//puts("Getting ready to call hashmap_get.");
  	struct tcp_connection *connection = hashmap_get(&tcp_map, &socket);
	if (connection == NULL) {
	  puts("Did not find existing connection.");
	  return;
	}
	if (source_is_greater > 0) {
	  if (tcp.window_scale_exists) {
            connection->peer_b_window_scale = tcp.window_scale;
            connection->peer_b_offered_window_scaling = 1;
	  }
	} else {
	  if (tcp.window_scale_exists) {
            connection->peer_a_window_scale = tcp.window_scale;
            connection->peer_a_offered_window_scaling = 1;
	  }
	}
	if (connection->peer_a_offered_window_scaling == 1 && connection->peer_b_offered_window_scaling == 1) {
	    connection->window_scaling_enabled = 1;
	    //printf("window scaling is enabled.\n");
	}
	if (connection == NULL) {
	  //printf("Socket not found in the hashmap.\n");
	} else {
	  //printf("The connection state of the retrieved socket is %d.\n", connection->state);
	  connection->state = HAL_TCP_STATE_SYN_RECEIVED;
	  ev_timer_stop(loop, &(connection->timeout_watcher));
	  //printf("peer a window scale = %u, peer b window scale = %u.\n", connection->peer_a_window_scale, connection->peer_b_window_scale);
	}
    }

    if(
      (tcp.flags & HAL_TCP_FLAG_FIN) &&
      (tcp.flags & HAL_TCP_FLAG_ACK) == 0) {
        //printf("FIN\n");
    }
    if(
      (tcp.flags & HAL_TCP_FLAG_FIN) &&
      (tcp.flags & HAL_TCP_FLAG_ACK)) {
        //printf("FIN ACK\n");
        struct tcp_socket socket;
	memset(&socket, 0, sizeof(struct tcp_socket));
        if (v4tcpsocketcmp(ipv4->source_ip, tcp.source_port, ipv4->destination_ip, tcp.destination_port) > 0) {
	  socket.peer_b_ip = ipv4->source_ip;
	  socket.peer_b_port = tcp.source_port;
	  socket.peer_a_ip = ipv4->destination_ip;
	  socket.peer_a_port = tcp.destination_port;
	} else {
	  socket.peer_a_ip = ipv4->source_ip;
	  socket.peer_a_port = tcp.source_port;
	  socket.peer_b_ip = ipv4->destination_ip;
	  socket.peer_b_port = tcp.destination_port;
	}
  	struct tcp_connection *connection = hashmap_get(&tcp_map, &socket);
	if (connection == NULL) {
	  //printf("Socket not found in the hashmap.\n");
	} else {
	  //printf("The connection state of the retrieved socket is %d.\n", connection->state);
          if (connection->peer_a_bytes > 25000 || connection->peer_b_bytes > 25000) {
	    printf("Connection closed. socket=%s-%s:%u-%u, peer a bytes=%llu, peer b bytes=%llu. Connections found=%llu, not found=%llu.\n",
              ipv4_source_ip_in_decimal (ipv4),
              ipv4_destination_ip_in_decimal (ipv4),
	      socket.peer_a_port,
	      socket.peer_b_port,
              connection->peer_a_bytes,
	      connection->peer_b_bytes,
	      connections_found,
	      connections_not_found
	    );
	    int i=0;
	    HASHMAP_ITER(tcp_map) it;
	    for (it = hashmap_iter(&tcp_map); hashmap_iter_valid(&it); hashmap_iter_next(&it)) {
	      i++;
	    }
	    printf("There are %d entries in the hashtable.\n", i);
	  }
	  ev_timer_stop(loop, &(connection->timeout_watcher));
	  struct tcp_connection *val = hashmap_remove(&tcp_map, &socket);
	  free(val);
	  //puts("Removed and deleted hashmap entry.");
	}
    }

    return HAL_TCP_OK;
}
