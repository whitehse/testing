#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <arpa/inet.h>
//#include <hal/ethernet.h>
#include <hal/ipv4.h>
#include <hal/udp.h>
#include <hashmap/hashmap.h>

typedef HASHMAP(char, int) udp_hash_t;

int my_hal_ipv4_listener(struct hal_ipv4 *ipv4) {
    struct hal_udp udp;
    static udp_hash_t udp_hash;
    static udp_init = 0;
    static uint64_t total_udp_bytes = 0;
    static int bad_segments = 0;

    if (udp_init == 0) {
        printf("udp is null\n");
        hashmap_init(&udp_hash, hashmap_hash_string, strcmp);
        hashmap_put(&udp_hash, "42", 43);
	udp_init = 1;
    }

    //int other_num = hashmap_get(&udp_hash, "42");
    //printf("The returned numnber is %d\n", other_num);

    if (ipv4->protocol != HAL_IPV4_PROT_UDP) {
	return HAL_UDP_NA;
    }

    char* buf = ipv4->payload_pointer;

    //printf("segment type = %d\n", ipv4->protocol);

    if (ipv4->payload_length < HAL_UDP_MIN_SEGMENT_SIZE || ipv4->payload_length > HAL_UDP_MAX_SEGMENT_SIZE) {
        printf("UDP segment had the wrong size\n");
	bad_segments += 1;
        return HAL_UDP_BADPARAM;
    }

    udp.source_port = ntohs(*(uint16_t*)buf);
    udp.destination_port = ntohs(*(uint16_t*)(buf+2));
    udp.payload_length = ntohs(*(uint16_t*)(buf+4));
    udp.checksum = ntohs(*(uint16_t*)(buf+6));
    udp.payload_pointer = buf+8;
    total_udp_bytes += udp.payload_length - 8;

/*
    printf("udp:source port=%d, destination port=%d, payload bytes=%d, total bytes=%lld, bad segments=%d\n",
        udp.source_port,
	udp.destination_port,
	udp.payload_length - 8,
	total_udp_bytes,
	bad_segments
    );
*/

    if (ipv4->payload_length != (udp.payload_length)) {
	bad_segments += 1;
        return HAL_UDP_BADPARAM;
    }

    //printf("The reported UDP payload size does not match the size reported by the IP layer (+8)\n");
    //printf("UDP length=%d, IP Payload Length=%d\n", udp.payload_length, ipv4->payload_length);
    //printf("Source port=%d, Destination port=%d\n", udp.source_port, udp.destination_port);

    return HAL_UDP_OK;
}
