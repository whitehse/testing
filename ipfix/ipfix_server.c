#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

#include <ev.h>
//#define EV_STANDALONE 1
//#include <ev.c>
//#include <cfgpath.h>
//#include <cbor.h>
#include <cjson/cJSON.h>

int counter;

#define EVE_CONNECTION_INIT        1
#define EVE_CONNECTION_ESTABLISHED 2

//cJSON* cbor_to_cjson(cbor_item_t* item) {
//  switch (cbor_typeof(item)) {
//    case CBOR_TYPE_UINT:
//      return cJSON_CreateNumber(cbor_get_int(item));
//    case CBOR_TYPE_NEGINT:
//      return cJSON_CreateNumber(-1 - cbor_get_int(item));
//    case CBOR_TYPE_BYTESTRING:
//      // cJSON only handles null-terminated string -- binary data would have to
//      // be escaped
//      return cJSON_CreateString("Unsupported CBOR item: Bytestring");
//    case CBOR_TYPE_STRING:
//      if (cbor_string_is_definite(item)) {
//        // cJSON only handles null-terminated string
//        char* null_terminated_string = malloc(cbor_string_length(item) + 1);
//        memcpy(null_terminated_string, cbor_string_handle(item),
//               cbor_string_length(item));
//        null_terminated_string[cbor_string_length(item)] = 0;
//        cJSON* result = cJSON_CreateString(null_terminated_string);
//        free(null_terminated_string);
//        return result;
//      }
//      return cJSON_CreateString("Unsupported CBOR item: Chunked string");
//    case CBOR_TYPE_ARRAY: {
//      cJSON* result = cJSON_CreateArray();
//      for (size_t i = 0; i < cbor_array_size(item); i++) {
//        cJSON_AddItemToArray(result, cbor_to_cjson(cbor_array_get(item, i)));
//      }
//      return result;
//    }
//    case CBOR_TYPE_MAP: {
//      cJSON* result = cJSON_CreateObject();
//      for (size_t i = 0; i < cbor_map_size(item); i++) {
//        char* key = malloc(128);
//        snprintf(key, 128, "Surrogate key %zu", i);
//        // JSON only support string keys
//        if (cbor_isa_string(cbor_map_handle(item)[i].key) &&
//            cbor_string_is_definite(cbor_map_handle(item)[i].key)) {
//          size_t key_length = cbor_string_length(cbor_map_handle(item)[i].key);
//          if (key_length > 127) key_length = 127;
//          // Null-terminated madness
//          memcpy(key, cbor_string_handle(cbor_map_handle(item)[i].key),
//                 key_length);
//          key[key_length] = 0;
//        }
//
//        cJSON_AddItemToObject(result, key,
//                              cbor_to_cjson(cbor_map_handle(item)[i].value));
//        free(key);
//      }
//      return result;
//    }
//    case CBOR_TYPE_TAG:
//      return cJSON_CreateString("Unsupported CBOR item: Tag");
//    case CBOR_TYPE_FLOAT_CTRL:
//      if (cbor_float_ctrl_is_ctrl(item)) {
//        if (cbor_is_bool(item)) return cJSON_CreateBool(cbor_get_bool(item));
//        if (cbor_is_null(item)) return cJSON_CreateNull();
//        return cJSON_CreateString("Unsupported CBOR item: Control value");
//      }
//      return cJSON_CreateNumber(cbor_float_get_float(item));
//  }
//
//  return cJSON_CreateNull();
//}

void hex_dump(char *desc, void *addr, int len) {
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

#define BUFFER_SIZE 1024

// https://www.iana.org/assignments/ipfix/ipfix.xhtml

//received 140 bytes from client. Packets seen: 161
//IPFIX Client: :
//  0000  00 0a 00 8c 67 99 2c c2 00 10 27 41 00 08 00 00  ....g.,...'A....
//  0010  00 02 00 7c 01 00 00 1d 00 08 00 04 00 0c 00 04  ...|............
//  0020  00 05 00 01 00 04 00 01 00 07 00 02 00 0b 00 02  ................
//  0030  00 20 00 02 00 0a 00 04 00 3a 00 02 00 09 00 01  . .......:......
//  0040  00 0d 00 01 00 10 00 04 00 11 00 04 00 0f 00 04  ................
//  0050  00 06 00 01 00 0e 00 04 00 34 00 01 00 35 00 01  .........4...5..
//  0060  00 88 00 01 00 3c 00 01 00 12 00 04 00 3d 00 01  .....<.......=..
//  0070  00 f3 00 02 00 f5 00 02 00 36 00 04 00 01 00 08  .........6......
//  0080  00 02 00 08 00 98 00 08 00 99 00 08              ............

// version = 00 0a (10)
// message length = 00 8c - 140
// timestamp = 67 99 2c c2 - Tue Jan 28 2025 13:15:14 GMT-0600 (Central Standard Time)
// sequence number = 00 10 27 41 - 1058625
// observation domain id = 00 08 00 00 - 524288
// Set ID (Template) = 00 02
// Data Set Length = 00 7c - 124
// Template ID = 01 00 - 256
// Number of fields = 00 1d - 29
// Field 1 Type: 00 08 - 8, sourceIPv4Address
// Field 1 Length: 00 04 - 4 bytes
// Field 2 Type: 00 0c - 12, destinationIPv4Address
// Field 2 Length: 00 04 - 4 bytes
// Field 3 Type: 00 05 - 5, ipClassOfService
// Field 3 Length: 00 01 - 1 byte
// Field 4 Type: 00 04 - 4, protocolIdentifier
// Field 4 Length: 00 01 - 1 byte
// Field 5 Type: 00 07 - 7, sourceTransportPort
// Field 5 Length: 00 02, 2 bytes
// Field 6 Type: 00 0b - destinationTransportPort
// Field 6 Length: 00 02 - 2 bytes
// Field 7 Type: 00 20, icmpTypeCodeIPv4
// Field 7 Length: 00 02 - 2 bytes
// Field 8 Type: 00 0a - ingressInterface
// Field 8 Length: 00 04 - 4 bytes
// Field 9 Type: 00 3a - vlanId
// Field 9 Length: 00 02 - 2 bytes
// Field 10 Type: 00 09 - sourceIPv4PrefixLength
// Field 10 Length: 00 01 - 1 byte
// Field 11 Type: 00 0d - destination IPv4PrefixLength
// Field 11 Length: 00 01 - 1 byte
// Field 12 Type: 00 10 - bgpSourceAsNumber
// Field 12 Length: 00 04 - 4 bytes
// Field 13 Type: 00 11 - bgpDestinationAsNumber
// Field 13 Length: 00 04 - 4 bytes
// Field 14 Type: 00 0f - ipnextHopIPv4Address
// Field 14 Length: 00 04 - 4 bytes
// Field 15 Type: 00 06 - tcpControlBits
// Field 15 Length: 00 01 - 1 byte
// Field 16 Type: 00 0e - egreeeInterface
// Field 16 Length: 00 04 - 4 bytes
// Field 17 Type: 00 34 - minimumTTL
// Field 17 Length: 00 01 - 1 byte
// Field 18 Type: 00 35 - maximumTTL
// Field 18 Length: 00 01 - 1 byte
// Field 19 Type: 00 88 - flowEndReason
// Field 19 Length: 00 01 - 1 byte
// Field 20 Type: 00 3c - ipVersion
// Field 20 Length: 00 01 - 1 byte
// Field 21 Type: 00 12 - bgpNextHopIPv4Address
// Field 21 Length: 00 04 - 4 bytes
// Field 22 Type: 00 3d - flowDirection
// Field 22 Length: 00 01 - 1 byte
// Field 23 Type: 00 f3 - dog1qVlanId
// Field 23 Length: 00 02 - 2 bytes
// Field 24 Type: 00 f5 - dot1qCustomerVlanId
// Field 24 Length: 00 02 - 2 bytes
// Field 25 Type: 00 36 - fragmentationIdentification
// Field 25 Length: 00 04 - 4 bytes
// Field 26 Type: 00 01 - octetDeltaCount
// Field 26 Length: 00 08 - 8 bytes
// Field 27 Type: 00 02 - packetDeltaCount
// Field 27 Length: 00 08 - 8 bytes
// Field 28 Type: 00 98 - flowStartMilliseconds
// Field 28 Length: 00 08 - 8 bytes
// Field 29 Type: 00 99 - flowEndMilliseconds
// Field 29 Length: 00 08 - 8 bytes


//received 72 bytes from client. Packets seen: 162
//IPFIX Client: :
//  0000  00 0a 00 48 67 99 2c c2 00 00 00 67 00 08 00 00  ...Hg.,....g....
//  0010  00 03 00 38 02 00 00 0b 00 01 00 90 00 04 00 29  ...8...........)
//  0020  00 08 00 2a 00 08 00 a0 00 08 00 82 00 04 00 83  ...*............
//  0030  00 10 00 22 00 04 00 24 00 02 00 25 00 02 00 d6  ..."...$...%....
//  0040  00 01 00 d7 00 01 00 00                          ........
//
// version = 00 0a (10)
// message length = 00 48 - 72
// timestamp = 67 99 2c c2 - Tue Jan 28 2025 13:15:14 GMT-0600 (Central Standard Time)
// sequence number = 00 00 00 67 - 86
// observation domain id = 00 08 00 00 - 524288
// Set ID (Template) = 00 03
// Data Set Length = 00 38 - 56
// Template ID = 02 00 - 512
// Field Count = 00 0b - 11
// Scope Field Count = 00 01 - 1
// Scope 1 Information Element id = 00 90 - 144, exportingProcessId
// Scope 1 Field Length = 00 04 - 4 bytes
// Scope 2 Information Element id = 00 29 = exportedOctetTotalCount
// Scope 2 Field Length = 00 08 - 8 bytes
// Scope 3 Information Element id = 00 2a = exportedFlowRecordTotalCount
// Scope 3 Field Length = 00 08 - 8 bytes
// Scope 4 Information Element id = 00 a0 = 160, systemInitTimeMilliseconds
// Scope 4 Field Length = 00 08 - 8 bytes
// Scope 5 Information Element id = 00 82 - 130, exporterIpv4Address
// Scope 5 Field Length = 00 04 - 4 bytes
// Scope 6 Information Element id = 00 83 - exporterIpv6Address
// Scope 6 Field Length = 00 10 - 16 bytes
// Scope 7 Information Element id = 00 22 - samplingInterval
// Scope 7 Field Length = 00 04 - 4 bytes
// Scope 8 Information Element id = 00 24 - flowActiveTimeout
// Scope 8 Field Length = 00 02 - two bytes
// Scope 9 Information Element id = 00 25 - flowIdleTimeout
// Scope 9 Field Length = 

// Field 1 Type: 00 01 - octetDeltaCount
// Field 1 Length: 00 08 - 8 bytes

// Field 1 Type: 00 08 - 8, sourceIPv4Address
// Field 1 Length: 00 04 - 4 bytes
// Field 2 Type: 00 0c - 12, destinationIPv4Address
// Field 2 Length: 00 04 - 4 bytes
// Field 3 Type: 00 05 - 5, ipClassOfService
// Field 3 Length: 00 01 - 1 byte
// Field 4 Type: 00 04 - 4, protocolIdentifier
// Field 4 Length: 00 01 - 1 byte
// Field 5 Type: 00 07 - 7, sourceTransportPort
// Field 5 Length: 00 02, 2 bytes
// Field 6 Type: 00 0b - destinationTransportPort
// Field 6 Length: 00 02 - 2 bytes
// Field 7 Type: 00 20, icmpTypeCodeIPv4
// Field 7 Length: 00 02 - 2 bytes
// Field 8 Type: 00 0a - ingressInterface
// Field 8 Length: 00 04 - 4 bytes
// Field 9 Type: 00 3a - vlanId
// Field 9 Length: 00 02 - 2 bytes
// Field 10 Type: 00 09 - sourceIPv4PrefixLength
// Field 10 Length: 00 01 - 1 byte
// Field 11 Type: 00 0d - destination IPv4PrefixLength
// Field 11 Length: 00 01 - 1 byte
// Field 12 Type: 00 10 - bgpSourceAsNumber
// Field 12 Length: 00 04 - 4 bytes
// Field 13 Type: 00 11 - bgpDestinationAsNumber
// Field 13 Length: 00 04 - 4 bytes
// Field 14 Type: 00 0f - ipnextHopIPv4Address
// Field 14 Length: 00 04 - 4 bytes
// Field 15 Type: 00 06 - tcpControlBits
// Field 15 Length: 00 01 - 1 byte
// Field 16 Type: 00 0e - egreeeInterface
// Field 16 Length: 00 04 - 4 bytes
// Field 17 Type: 00 34 - minimumTTL
// Field 17 Length: 00 01 - 1 byte
// Field 18 Type: 00 35 - maximumTTL
// Field 18 Length: 00 01 - 1 byte
// Field 19 Type: 00 88 - flowEndReason
// Field 19 Length: 00 01 - 1 byte
// Field 20 Type: 00 3c - ipVersion
// Field 20 Length: 00 01 - 1 byte
// Field 21 Type: 00 12 - bgpNextHopIPv4Address
// Field 21 Length: 00 04 - 4 bytes
// Field 22 Type: 00 3d - flowDirection
// Field 22 Length: 00 01 - 1 byte
// Field 23 Type: 00 f3 - dog1qVlanId
// Field 23 Length: 00 02 - 2 bytes
// Field 24 Type: 00 f5 - dot1qCustomerVlanId
// Field 24 Length: 00 02 - 2 bytes
// Field 25 Type: 00 36 - fragmentationIdentification
// Field 25 Length: 00 04 - 4 bytes
// Field 26 Type: 00 01 - octetDeltaCount
// Field 26 Length: 00 08 - 8 bytes
// Field 27 Type: 00 02 - packetDeltaCount
// Field 27 Length: 00 08 - 8 bytes
// Field 28 Type: 00 98 - flowStartMilliseconds
// Field 28 Length: 00 08 - 8 bytes
// Field 29 Type: 00 99 - flowEndMilliseconds
// Field 29 Length: 00 08 - 8 bytes
//
//  0000  00 0a 01 7c 67 99 27 a7 00 09 4d 7b 00 08 00 00  ...|g.'...M{....
//  0010  01 00 01 6c 4a 7d 03 29 ae 80 81 66 00 06 01 bb  ...lJ}.)...f....
//  0020  d5 dc 00 00 00 00 02 5c 00 b5 14 1b 00 00 3b 41  .......\......;A
//  0030  00 06 10 c0 ae 80 81 be 10 00 00 02 67 7a 7a 02  ............gzz.
//  0040  04 ae 80 81 01 ff 00 00 00 00 00 00 00 00 00 00  ................
//  0050  00 00 00 00 21 f0 00 00 00 00 00 00 00 06 00 00  ....!...........
//  0060  01 94 ae 41 f7 00 00 00 01 94 ae 42 aa 00 b8 1c  ...A.......B....
//  0070  29 05 ae 80 9c 14 00 06 01 bb f5 9d 00 00 00 00  )...............
//  0080  02 5c 00 b5 18 20 00 00 51 cc 00 06 10 c0 ae 80  .\... ..Q.......
//  0090  81 85 18 00 00 02 d8 37 37 02 04 ae 80 81 0c ff  .......77.......
//  00a0  00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 63  ...............c
//  00b0  00 00 00 00 00 00 00 01 00 00 01 94 ae 41 ee 00  .............A..
//  00c0  00 00 01 94 ae 41 ee 00 89 dc 31 a3 40 e3 fa 06  .....A....1.@...
//  00d0  00 06 01 bb cd c8 00 00 00 00 02 5c 00 b5 16 20  ...........\... 
//  00e0  00 00 4f f9 00 06 10 c0 ae 80 81 be 10 00 00 02  ..O.............
//  00f0  67 36 36 02 04 ae 80 81 15 ff 00 00 00 00 00 00  g66.............
//  0100  00 00 00 00 00 00 00 00 00 34 00 00 00 00 00 00  .........4......
//  0110  00 01 00 00 01 94 ae 41 f7 00 00 00 01 94 ae 41  .......A.......A
//  0120  f7 00 68 12 07 94 d1 5f 77 91 00 06 01 bb e4 2b  ..h...._w......+
//  0130  00 00 00 00 02 5c 00 b5 14 20 00 00 34 17 00 06  .....\... ..4...
//  0140  10 c0 ae 80 81 85 18 00 00 02 d8 3a 3a 02 04 ae  ...........::...
//  0150  80 81 0e ff 00 00 00 00 00 00 00 00 00 00 00 00  ................
//  0160  00 00 05 dc 00 00 00 00 00 00 00 01 00 00 01 94  ................
//  0170  ae 41 fc 00 00 00 01 94 ae 41 fc 00              .A.......A..

// version = 00 0a (10)
// message length = 01 7c - 380 bytes
// timestamp = 67 99 27 a7 - Tue Jan 28 2025 12:53:27 GMT-0600 (Central Standard Time)
// sequence number = 00 09 4d 7b - 609659
// observation domain id = 00 08 00 00 - 524288
// Data Set = 01 00 - 256 - Data Set
// Data Set Length = 01 6c - 364 bytes


void echo_read_cb(struct ev_loop *loop, struct ev_io *w, int revents){
  char buffer[BUFFER_SIZE];
  //char buffer[1024];
  ssize_t read;

  //connection_t *connection = w->data;

  //hex_dump(NULL, client_initial_key, 32);

  if(EV_ERROR & revents) {
    perror("got invalid event");
    return;
  }

  memset(buffer, 0, sizeof(buffer));
  //int size = read(w->fd, buffer, sizeof(buffer));
  read = recv(w->fd, buffer, BUFFER_SIZE, 0);
  if ( read < 0 ) {
    perror("read error");
    exit(5);
  }

  if (read == 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      perror("Something weird happened");
      return 0;
    } else {
      puts("Socket was closed");
      ev_io_stop(loop, w);
      close(w->fd);
      free(w);
      return 0;
    }
  }

  counter = counter + 1;
  printf("received %d bytes from client. Packets seen: %d\n", read, counter);
  hex_dump("IPFIX Client: ", buffer, read);
}

void echo_accept_cb(struct ev_loop *loop, struct ev_io *w, int revents) {
  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);
  int client_sd;
  struct ev_io *w_client = (struct ev_io*) malloc (sizeof(struct ev_io));

  if(EV_ERROR & revents) {
    perror("got invalid event");
    return;
  }

  client_sd = accept(w->fd, (struct sockaddr *)&client_addr, &client_len);

  if (client_sd < 0) {
    perror("accept error");
    return;
  }

  printf("Successfully connected with client.\n");

  ev_io_init(w_client, echo_read_cb, client_sd, EV_READ);
  ev_io_start(loop, w_client);
}

int main(int argc, char const *argv[]) {
  struct ev_loop *loop = ev_default_loop(0);
  counter = 0;

  int echo_server_fd;
  struct sockaddr_in server, client;
  int len;
  int echo_port = 1235;
  //echo_server_fd = socket(AF_INET, SOCK_STREAM, 0);
  echo_server_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (echo_server_fd < 0) {
    perror("Cannot create socket");
    exit(1);
  }
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(echo_port);
  len = sizeof(server);
  if (bind(echo_server_fd, (struct sockaddr *)&server, len) < 0) {
    perror("Cannot bind sokcet");
    exit(2);
  }
  //if (listen(echo_server_fd, 100000) < 0) {
  //  perror("Listen error");
  //  exit(3);
  //}

  struct ev_io *w_accept = (struct ev_io*) malloc (sizeof(struct ev_io));
  //ev_io_init(w_accept, echo_accept_cb, echo_server_fd, EV_READ);
  ev_io_init(w_accept, echo_read_cb, echo_server_fd, EV_READ);
  ev_io_start(loop, w_accept);

  ev_run (loop, 0);

  return 0;
}

