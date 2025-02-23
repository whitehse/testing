#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

#include <ev.h>
#include <cbor.h>
#include <cjson/cJSON.h>
#include <hashmap.h>

#include <ipfix_iana.h>

// Calix AXOS:
//{
//   "flow_generator_ipv4_address" : "174.128.129.0",
//   "version" : 10
//   "message_length" : 1056,
//   "timestamp" : 1740257779,
//   "observation_domain_id" : 0,
//   "sequence_number" : 8537,
//   "templates" : [
//      {
//         "template_id" : 286,
//         "number_of_fields" : 22,
//         "fields" : [ 956, 1, 130, 56, 602, 978, 955, 603, 4, 206, 207, 204, 205, 631, 10, 2, 3, 202, 203, 200, 201, 632 ]
//      },
//      {
//         "template_id" : 287,
//         "number_of_fields" : 19,
//         "fields" : [ 956, 1, 131, 56, 602, 978, 955, 603, 206, 207, 204, 205, 631, 10, 202, 203, 200, 201, 632 ]
//      },
//      {
//         "template_id" : 288,
//         "number_of_fields" : 16,
//         "fields" : [ 106, 20, 252, 256, 259, 258, 1, 56, 7, 22, 25, 240, 247, 250, 249, 242 ]
//      },
//      {
//         "template_id" : 289,
//         "number_of_fields" : 8,
//         "fields" : [ 905, 12, 1, 56, 7, 1214, 10, 904 ]
//      },
//      {
//         "template_id" : 290,
//         "number_of_fields" : 13,
//         "fields" : [ 105, 1, 233, 56, 232, 984, 7, 985, 231, 10, 234, 983, 987 ]
//      },
//      {
//         "template_id" : 291,
//         "number_of_fields" : 38,
//         "fields" : [ 103, 956, 320, 212, 210, 323, 321, 213, 211, 209, 324, 322, 1, 56, 955, 4, 982, 622, 206, 207, 621, 205, 620, 10, 2, 3, 234, 988, 981, 627, 202, 203, 626, 201, 979, 625, 980, 208 ]
//      },
//      {
//         "template_id" : 292,
//         "number_of_fields" : 7,
//         "fields" : [ 905, 1, 56, 7, 903, 10, 904 ]
//      }
//   ],
//   "flow_sets" : []
//}
//

/* Maximum packet size for flow packets. Flow packets, and in particular
 * UDP flow packets, are unlikely to be more than 1500 bytes */
/* Actually, the E7 AXOS flow source sends larger packets */
#define PACKET_BUFFER_SIZE 50000
int counter;

//typedef HASHMAP(struct tcp_socket, struct tcp_connection) tcp_hash_t;
//tcp_hash_t tcp_map;

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
        cJSON_AddItemToArray(result, cbor_to_cjson(cbor_move(cbor_array_get(item, i))));
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

// https://www.iana.org/assignments/ipfix/ipfix.xhtml

//received 140 bytes from client.
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
// Set ID (Template) = 00 02 (Template Set Definition)
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

//received 72 bytes from client.
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
// Set ID (Template) = 00 03 (Options Template Set Definition)
// Data Set Length = 00 38 - 56
// Template ID = 02 00 - 512
// Field Count = 00 0b - 11
// Scope Field Count = 00 01 - 1 (must not be zero. What happens when its greater than one?)
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
// Scope 9 Field Length = 00 02 - 2 bytes
// Scope 10 Information Element id = 00 d6 - exportProtocolVersion
// Scope 10 Field Length = 00 01 = 1 byte
// Scope 11 Information Element id = 00 d7 - exportTransportProtocol
// Scope 11 Field Length = 00 01 = 1 byte 
// Padding (to 4 byte boundary) = 00 00

// Flow record packet:
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
// Set ID = 01 00 - 256 (Data Set)
// Data Set Length = 01 6c - 364 bytes

// Record 1
  // Field 1 Value: 4a 7d 03 29 - SourceIPv4Address, 74.125.3.41
  // Field 2 Value: ae 80 81 66 - DestinationIPv4Address, 174.128.129.102
  // Field 3 Value: 00 - ipClassOfService, 0
  // Field 4 Type: 06 - protocolIdentifier, TCP
  // Field 5 Type: 01 bb - sourceTransportPort, 443
  // Field 6 Type: d5 dc - destinationTransportPort, 54748
  // Field 7 Type: 00 00 - icmpTypeCodeIPv4, 0, ICMP Reply (?)
  // Field 8 Type: 00 00 02 5c - ingressInterface, 604
  // Field 9 Type: 00 b5 - vlanId, 181
  // Field 10 Type: 14 - sourceIPv4PrefixLength, 20
  // Field 11 Type: 1b - destination IPv4PrefixLength, 27
  // Field 12 Type: 00 00 3b 41 - bgpSourceAsNumber, 15169
  // Field 13 Type: 00 06 10 c0 - bgpDestinationAsNumber, 397504
  // Field 14 Type: ae 80 81 be - ipnextHopIPv4Address, 174.128.129.190
  // Field 15 Type: 10 - tcpControlBits, 16
  // Field 16 Type: 00 00 02 67 - egressInterface, 615
  // Field 17 Type: 7A - minimumTTL, 122
  // Field 18 Type: 7A - maximumTTL, 122
  // Field 19 Type: 02 - flowEndReason, 2, active timeout
  // Field 20 Type: 04 - ipVersion, 4 (IPv4, taken from IP version field in the packet header)
  // Field 21 Type: ae 80 81 01 - bgpNextHopIPv4Address, 174.128.129.1
  // Field 22 Type: ff - flowDirection, Invalid, must be 0x00 or 0x01 per the spec
  // Field 23 Type: 00 00 - dog1qVlanId, 0
  // Field 24 Type: 00 00 - dot1qCustomerVlanId, 0
  // Field 25 Type: 00 00 00 00 - fragmentationIdentification, 0
  // Field 26 Type: 00 00 00 00 00 00 21 f0 - octetDeltaCount, 8688
  // Field 27 Type: 00 00 00 00 00 00 00 06 - packetDeltaCount, 6
  // Field 28 Type: 00 00 01 94 ae 41 f7 00 - flowStartMilliseconds - Tue Jan 28 2025 12:52:26 GMT-0600 (Central Standard Time) and 240 milliseconds
  // Field 29 Type: 00 00 01 94 ae 42 aa 00 - flowEndMilliseconds - Tue Jan 28 2025 12:53:12 GMT-0600 (Central Standard Time) and 64 milliseconds
// Record 2
  // Field 1 Value: b8 1c 29 05 - SourceIPv4Address, 184.28.41.5
  // Field 2 Value: ae 80 9c 14 - DestinationIPv4Address, 174.128.156.20
  // ...

// Calix E7 AXOS
// https://datatracker.ietf.org/doc/html/draft-boydseda-ipfix-psamp-bulk-data-yang-model-03 ?
//Set definition, from 174.128.129.0: :
//  0000  00 0a 04 20 67 ba 24 db 00 00 0b 33 00 00 00 00  ... g.$....3....
//  0010  00 02 00 b8 01 1e 00 16 83 bc ff ff 00 00 18 b1  ................
//  0020  80 01 ff ff 00 00 18 b1 80 82 ff ff 00 00 18 b1  ................
//  0030  80 38 00 11 00 00 18 b1 82 5a 00 08 00 00 18 b1  .8.......Z......
//  0040  83 d2 ff ff 00 00 18 b1 83 bb ff ff 00 00 18 b1  ................
//  0050  82 5b 00 08 00 00 18 b1 80 04 ff ff 00 00 18 b1  .[..............

// version = 00 0a (10)
// message length = 04 20 - 1056
// timestamp = 67 ba 24 db - Sat Feb 22 2025 13:26:19 GMT-0600 (Central Standard Time)
// sequence number = 00 00 0b 33 - 2867
// observation domain id = 00 00 00 00 - 0
// Set ID (Template) = 00 02 (Template Set Definition)
// Data Set Length = 00 b8 - 184
// Template ID = 01 1e - 286
// Number of fields = 00 16 - 22
// Field 1 Type: 83 bc - 33724, 
// Field 1 Length: 00 04 - 4 bytes
// Field 2 Type: 00 0c - 12, destinationIPv4Address
// Field 2 Length: 00 04 - 4 bytes

//  0060  80 ce 00 08 00 00 18 b1 80 cf 00 08 00 00 18 b1  ................
//  0070  80 cc 00 08 00 00 18 b1 80 cd 00 08 00 00 18 b1  ................
//  0080  82 77 00 01 00 00 18 b1 80 0a 00 04 00 00 18 b1  .w..............
//  0090  80 02 00 01 00 00 18 b1 80 03 00 01 00 00 18 b1  ................
//  00a0  80 ca 00 08 00 00 18 b1 80 cb 00 08 00 00 18 b1  ................
//  00b0  80 c8 00 08 00 00 18 b1 80 c9 00 08 00 00 18 b1  ................
//  00c0  82 78 00 01 00 00 18 b1 00 02*00 a0 01 1f 00 13  .x..............
//  00d0  83 bc ff ff 00 00 18 b1 80 01 ff ff 00 00 18 b1  ................
//  00e0  80 83 ff ff 00 00 18 b1 80 38 00 11 00 00 18 b1  .........8......
//  00f0  82 5a 00 08 00 00 18 b1 83 d2 ff ff 00 00 18 b1  .Z..............
//  0100  83 bb ff ff 00 00 18 b1 82 5b 00 08 00 00 18 b1  .........[......
//  0110  80 ce 00 08 00 00 18 b1 80 cf 00 08 00 00 18 b1  ................
//  0120  80 cc 00 08 00 00 18 b1 80 cd 00 08 00 00 18 b1  ................
//  0130  82 77 00 01 00 00 18 b1 80 0a 00 04 00 00 18 b1  .w..............
//  0140  80 ca 00 08 00 00 18 b1 80 cb 00 08 00 00 18 b1  ................
//  0150  80 c8 00 08 00 00 18 b1 80 c9 00 08 00 00 18 b1  ................
//  0160  82 78 00 01 00 00 18 b1 00 02 00 88 01 20 00 10  .x........... ..
//  0170  80 6a ff ff 00 00 18 b1 80 14 ff ff 00 00 18 b1  .j..............
//  0180  80 fc 00 08 00 00 18 b1 81 00 00 08 00 00 18 b1  ...............

void flow_read_cb(struct ev_loop *loop, struct ev_io *w, int revents){
  char buffer[PACKET_BUFFER_SIZE];
  ssize_t read;

  if(EV_ERROR & revents) {
    perror("got invalid event");
    return;
  }

  memset(buffer, 0, sizeof(buffer));
  struct sockaddr_in client_addr;
  int addr_len = sizeof(struct sockaddr);
  read = recvfrom(w->fd, buffer, PACKET_BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
  if ( read < 0 ) {
    perror("read error");
    exit(5);
  }

  int res = getpeername(w->fd, (struct sockaddr *)&client_addr, &addr_len);
  char flow_generator_ipv4_address[20];
  strcpy(flow_generator_ipv4_address, inet_ntoa(client_addr.sin_addr));

  if (read == 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      perror("Something weird happened");
      return;
    } else {
      puts("Socket was closed");
      ev_io_stop(loop, w);
      close(w->fd);
      free(w);
      return;
    }
  }

  int has_templates = 0;
  /* The header is always 16 bytes long */
  if (read < 16) {
    return;
  }

  uint16_t version = ntohs(*(uint16_t *) (buffer+0));
  uint16_t message_length = ntohs(*(uint16_t *) (buffer+2));
  uint32_t message_timestamp = ntohl(*(uint32_t *) (buffer+4));
  uint32_t sequence_number = ntohl(*(uint32_t *) (buffer+8));
  uint32_t observation_domain_id = ntohl(*(uint32_t *) (buffer+12));
  /* Something went wrong between the OS/Network and the contents reported in the packet */
  /* This will need to be updated for future versions of ipfix */
  if (version != 10 || message_length != read) {
    printf("version is %d, and message length is %d, although %d was read from the socket.\n", version, message_length, read);
    return;
  }

  /* libcbor seems to be a nicely designed library and appropriate for
   * storing data that doesn't have a strict compile-time-known format.
   * The resulting data structure will represent a binary form of structured
   * data called CBOR (RFC 8949), which is easily conveted from and to JSON
   * for human readability and can be efficiently stored on disk or transmitted
   * over the network in a fairly efficient binary format */
  cbor_item_t* root = cbor_new_indefinite_map();
  if (root == NULL) {
    return;
  }
  bool success = cbor_map_add(root, (struct cbor_pair){
    .key = cbor_move(cbor_build_string("flow_generator_ipv4_address")),
    .value = cbor_move(cbor_build_string(flow_generator_ipv4_address))});
  success &= cbor_map_add(root, (struct cbor_pair){
    .key = cbor_move(cbor_build_string("version")),
    .value = cbor_move(cbor_build_uint16(version))});
  success &= cbor_map_add(root, (struct cbor_pair){
    .key = cbor_move(cbor_build_string("message_length")),
    .value = cbor_move(cbor_build_uint16(message_length))});
  success &= cbor_map_add(root, (struct cbor_pair){
    .key = cbor_move(cbor_build_string("timestamp")),
    .value = cbor_move(cbor_build_uint32(message_timestamp))});
  success &= cbor_map_add(root, (struct cbor_pair){
    .key = cbor_move(cbor_build_string("sequence_number")),
    .value = cbor_move(cbor_build_uint32(sequence_number))});
  success &= cbor_map_add(root, (struct cbor_pair){
    .key = cbor_move(cbor_build_string("observation_domain_id")),
    .value = cbor_move(cbor_build_uint32(observation_domain_id))});

  /* The message_length includes the message header or, in other words, the
   * message_length is the length of the entire packet */
  /* Template definitions have an ID of 2. These are sent periodically
   * from the flow source and subsequent flow records (ID >= 256) will
   * reference the defined template by ID */
  int bytes_remaining_in_packet = message_length - 16;
  int buffer_offset = 16;
  cbor_item_t* array_of_templates = cbor_new_indefinite_array();
  if (array_of_templates == NULL) {
    cbor_decref(&root);
    return;
  }
  success &= cbor_map_add(root, (struct cbor_pair){
    .key = cbor_move(cbor_build_string("templates")),
    .value = cbor_move(array_of_templates)});
  cbor_item_t* array_of_flow_sets = cbor_new_indefinite_array();
  if (array_of_flow_sets == NULL) {
    cbor_decref(&root);
    return;
  }
  success &= cbor_map_add(root, (struct cbor_pair){
    .key = cbor_move(cbor_build_string("flow_sets")),
    .value = cbor_move(array_of_flow_sets)});
  /* A set will be at least 4 bytes long (for the set header) even if there is
   * no actual content within the set. We will deal with padding within the
   * loop */
  while (success && bytes_remaining_in_packet >= 4) {
    uint16_t set_id = ntohs(*(uint16_t *) (buffer+buffer_offset));
    uint16_t set_length = ntohs(*(uint16_t *) (buffer+buffer_offset+2));
    /* The set length scrolls off the screen. Bail out */
    if (set_length > bytes_remaining_in_packet || set_length < 4) {
      printf("The set_length scrolls off the screen\n");
      cbor_decref(&root);
      return;
    }
    int bytes_remaining_in_set = set_length - 4;
    int buffer_offset_for_this_set = buffer_offset + 4;
    if (success && set_id == 2) { /* template definition */
      // Set ID (Template) = 00 02
      // Data Set Length = 00 7c - 124
      /* There must be a template ID and field length if a template definition is encountered */
      has_templates = 1;
      if (bytes_remaining_in_set < 4) {
        printf("A template definition was encountered, without ID and number of fields being defined\n");
        cbor_decref(&root);
        return;
      }
      cbor_item_t* template = cbor_new_indefinite_map();
      if (template == NULL) {
        cbor_decref(&root);
        return;
      }
      success &= cbor_array_push(array_of_templates, cbor_move(template));
      // Template ID = 01 00 - 256
      // Number of fields = 00 1d - 29
      uint16_t template_id = ntohs(*(uint16_t *) (buffer+buffer_offset_for_this_set));
      uint16_t number_of_fields = ntohs(*(uint16_t *) (buffer+buffer_offset_for_this_set+2));
      success &= cbor_map_add(template, (struct cbor_pair){
        .key = cbor_move(cbor_build_string("template_id")),
        .value = cbor_move(cbor_build_uint16(template_id))});
      success &= cbor_map_add(template, (struct cbor_pair){
        .key = cbor_move(cbor_build_string("number_of_fields")),
        .value = cbor_move(cbor_build_uint16(number_of_fields))});
      bytes_remaining_in_set -= 4;
      buffer_offset_for_this_set += 4;
      int number_of_fields_left_to_process = number_of_fields;
      cbor_item_t* array_of_fields = cbor_new_indefinite_array();
      if (array_of_fields == NULL) {
        cbor_decref(&root);
        return;
      }
      success &= cbor_map_add(template, (struct cbor_pair){
        .key = cbor_move(cbor_build_string("fields")),
        .value = cbor_move(array_of_fields)});
      /* A field definition should be at least 4 bytes */
      while (success && number_of_fields_left_to_process > 0 && bytes_remaining_in_set >= 4) {
        uint16_t field_type = ntohs(*(uint16_t *) (buffer+buffer_offset_for_this_set));
        uint16_t field_length = ntohs(*(uint16_t *) (buffer+buffer_offset_for_this_set+2));
        cbor_item_t* field_map = cbor_new_indefinite_map();
        if (field_map == NULL) {
          cbor_decref(&root);
          return;
        }
        success &= cbor_array_push(array_of_fields, cbor_move(field_map));
        success &= cbor_map_add(field_map, (struct cbor_pair){
          .key = cbor_move(cbor_build_string("field_length")),
          .value = cbor_move(cbor_build_uint16(field_length))});
        // Is this an enterprise-specific element?
        if (field_type > 32767) { 
          // There will be 4 additional bytes for the enterprise number
          if (bytes_remaining_in_set < 8) {
            cbor_decref(&root);
            return;
          }
          field_type -= 32768;
          uint32_t enterprise_number = ntohs(*(uint32_t *) (buffer+buffer_offset_for_this_set+4));
          success &= cbor_map_add(field_map, (struct cbor_pair){
            .key = cbor_move(cbor_build_string("enterprise_number")),
            .value = cbor_move(cbor_build_uint32(enterprise_number))});
          bytes_remaining_in_set -= 8;
          buffer_offset_for_this_set += 8;
        } else {
          bytes_remaining_in_set -= 4;
          buffer_offset_for_this_set += 4;
        }
        success &= cbor_map_add(field_map, (struct cbor_pair){
          .key = cbor_move(cbor_build_string("field_type")),
          .value = cbor_move(cbor_build_uint16(field_type))});
        //if (field_type <= number_of_iana_ipfix_elements) {
        //  success &= cbor_array_push(array_of_fields, cbor_move(cbor_build_uint16(field_type)));
        //} else {
        //  printf("Field Type %d is out of range\n", field_type);
        //}
        // Field 1 Type: 00 08 - 8, sourceIPv4Address
        // Field 1 Length: 00 04 - 4 bytes
        // Field 2 Type: 00 0c - 12, destinationIPv4Address
        // Field 2 Length: 00 04 - 4 bytes
        number_of_fields_left_to_process -= 1;
      }
    } else if (success && set_id == 3) { /* */
//      if (bytes_remaining_in_set < 4) {
//        printf("A template definition was encountered, without ID and number of fields being defined\n");
//        cbor_decref(&root);
//        return;
//      }
//      cbor_item_t* template = cbor_new_indefinite_map();
//      if (template == NULL) {
//        cbor_decref(&root);
//        return;
//      }
//      success &= cbor_array_push(array_of_templates, cbor_move(template));
//      // Template ID = 01 00 - 256
//      // Number of fields = 00 1d - 29
//      uint16_t template_id = ntohs(*(uint16_t *) (buffer+buffer_offset_for_this_set));
//      uint16_t number_of_fields = ntohs(*(uint16_t *) (buffer+buffer_offset_for_this_set+2));
//      success &= cbor_map_add(template, (struct cbor_pair){
//        .key = cbor_move(cbor_build_string("template_id")),
//        .value = cbor_move(cbor_build_uint16(template_id))});
//      success &= cbor_map_add(template, (struct cbor_pair){
//        .key = cbor_move(cbor_build_string("number_of_fields")),
//        .value = cbor_move(cbor_build_uint16(number_of_fields))});
//      bytes_remaining_in_set -= 4;
//      buffer_offset_for_this_set += 4;
//      int number_of_fields_left_to_process = number_of_fields;
//      cbor_item_t* array_of_fields = cbor_new_indefinite_array();
//      if (array_of_fields == NULL) {
//        cbor_decref(&root);
//        return;
//      }
//      success &= cbor_map_add(template, (struct cbor_pair){
//        .key = cbor_move(cbor_build_string("fields")),
//        .value = cbor_move(array_of_fields)});
//      /* A field definition should be at least 4 bytes */
//      while (success && number_of_fields_left_to_process > 0 && bytes_remaining_in_set >= 4) {
//        uint16_t field_type = ntohs(*(uint16_t *) (buffer+buffer_offset_for_this_set));
//        uint16_t field_length = ntohs(*(uint16_t *) (buffer+buffer_offset_for_this_set+2));
//        if (field_type <= number_of_iana_ipfix_elements) {
//          //printf("Field Type, %d, Field String %s, Field Length, %d\n", field_type, iana_ipfix_elements[field_type].name, field_length);
//          //success &= cbor_array_push(array_of_fields, cbor_move(cbor_build_uint16(field_type)));
//        } else {
//          printf("Field Type %d is out of range\n", field_type);
//        }
//        // Field 1 Type: 00 08 - 8, sourceIPv4Address
//        // Field 1 Length: 00 04 - 4 bytes
//        // Field 2 Type: 00 0c - 12, destinationIPv4Address
//        // Field 2 Length: 00 04 - 4 bytes
//        bytes_remaining_in_set -= 4;
//        buffer_offset_for_this_set += 4;
//        number_of_fields_left_to_process -= 1;
//      }
    } else if (success && set_id >= 256) { /* flow record set */
      // Data Set = 01 00 - 256 - Data Set
      // Data Set Length = 01 6c - 364 bytes
      // Record 1
      // Field 1 Value: 4a 7d 03 29 - SourceIPv4Address, 74.125.3.41
      // Field 2 Value: ae 80 81 66 - DestinationIPv4Address, 174.128.129.102
      // Field 3 Value: 00 - ipClassOfService, 0
      // Field 4 Type: 06 - protocolIdentifier, TCP
    }
    bytes_remaining_in_packet -= set_length;
    buffer_offset += set_length;
  }
      /*
       {
        "flow_generator_ipv4_address": "174.128.129.4",
        "version": 10,
        "message_length": 140,
        "timestamp": 1739718327,
        "sequence_number": 311889030,
        "observation_domain_id": 524288,
        "templates": [
           {
              "template_id": 2,
              "number_of_fields": 4,
              fields: [ 8, 2, 7, 13 ]
           }
        ]
        "flow_sets":  [
          {
                "test": "value"
          }
        ]
       }
*/

  if (!success) {
    puts("\nBuilding CBOR failed");
    cbor_decref(&root);
    return;
  }

  //cbor_describe(root, stdout);
  unsigned char* cbor_buffer;
  size_t cbor_buffer_size;
  cbor_serialize_alloc(root, &cbor_buffer, &cbor_buffer_size);
  cJSON* cjson_item = cbor_to_cjson(root);
  if (strcmp(flow_generator_ipv4_address, "174.128.129.0") == 0 && has_templates == 1) {
    hex_dump("Set definition, from 174.128.129.0: ", buffer, read);
    char* json_string = cJSON_PrintUnformatted(cjson_item);
    printf("%s\n", json_string);
    free(json_string);
  }
  cJSON_Delete(cjson_item);
  free(cbor_buffer);
  cbor_decref(&root);

  counter = counter + 1;
  if (counter % 50000 == 0) {
    printf("\nPackets seen: %d\n", counter);
    FILE *fp;
    char path[1024];
    char line[1024];
    size_t size = 0;
    snprintf(path, sizeof(path), "/proc/%d/status", getpid());
    fp = fopen(path, "r");
    if (fp == NULL) {
      perror("Error opening status file");
      return;
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
      // Find the VmRSS line (Resident Set Size)
      if (strncmp(line, "VmRSS:", 6) == 0) {
        // Extract the memory usage value
        if (sscanf(line, "VmRSS: %zu kB", &size) == 1) {
          printf("Memory usage: %zu KB\n", size);
        }
        break; // Stop after finding VmRSS
      }
    }
    fclose(fp);
  }
  //hex_dump("IPFIX Client: ", buffer, read);
}

/* This is unused for UDP based ipfix collection. Keep in place
 * for TCP/SCTP flow collection if needed in the future */
void flow_accept_cb(struct ev_loop *loop, struct ev_io *w, int revents) {
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

  ev_io_init(w_client, flow_read_cb, client_sd, EV_READ);
  ev_io_start(loop, w_client);
}

int main(int argc, char const *argv[]) {
  struct ev_loop *loop = ev_default_loop(0);
  counter = 0;

  int flow_server_fd;
  struct sockaddr_in server, client;
  int len;
  int flow_port = 1235;
  //flow_server_fd = socket(AF_INET, SOCK_STREAM, 0);
  /* Listen on UDP */
  flow_server_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (flow_server_fd < 0) {
    perror("Cannot create socket");
    exit(1);
  }
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(flow_port);
  len = sizeof(server);
  if (bind(flow_server_fd, (struct sockaddr *)&server, len) < 0) {
    perror("Cannot bind socket");
    exit(2);
  }

  struct ev_io *w_accept = (struct ev_io*) malloc (sizeof(struct ev_io));
  /* Not needed for UDP based flow collection */
  //ev_io_init(w_accept, flow_accept_cb, flow_server_fd, EV_READ);
  ev_io_init(w_accept, flow_read_cb, flow_server_fd, EV_READ);
  ev_io_start(loop, w_accept);

  ev_run (loop, 0);

  return 0;
}
