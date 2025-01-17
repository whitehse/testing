#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <hal/ethernet.h>
#include <hal/ipv4.h>

//#define init hal_ipv4_LTX_init
//#define parse hal_ipv4_LTX_parse

char* ipv4_source_ip_in_decimal (struct hal_ipv4* ipv4) {
    snprintf(ipv4->source_ip_decimal, 16,
        "%d.%d.%d.%d",
        (unsigned char)((ipv4->source_ip & 0xff000000) >> 24),
        (unsigned char)((ipv4->source_ip & 0x00ff0000) >> 16),
        (unsigned char)((ipv4->source_ip & 0x0000ff00) >> 8),
        (unsigned char)(ipv4->source_ip & 0x000000ff));
    return ipv4->source_ip_decimal;
}

char* ipv4_destination_ip_in_decimal (struct hal_ipv4* ipv4) {
    snprintf(ipv4->destination_ip_decimal, 16,
        "%d.%d.%d.%d",
        (unsigned char)((ipv4->destination_ip & 0xff000000) >> 24),
        (unsigned char)((ipv4->destination_ip & 0x00ff0000) >> 16),
        (unsigned char)((ipv4->destination_ip & 0x0000ff00) >> 8),
        (unsigned char)(ipv4->destination_ip & 0x000000ff));
    return ipv4->destination_ip_decimal;
}

int my_hal_ethernet_listener(struct hal_ethernet *frame) {
    struct hal_ipv4 ipv4;
    static ipv4_init = 0;

    int i;
    int offset=0;
    int flags;

    if (ipv4_init == 0) {
        //printf("First time ethernet listener was called\n");
	ipv4_init = 1;
    }

    char* buf = frame->payload_pointer;
    ipv4.packet_size = frame->payload_length;
    ipv4.number_of_options = 0;

    if (ipv4.packet_size < HAL_IPV4_MIN_PACKET_SIZE || ipv4.packet_size > HAL_IPV4_MAX_PACKET_SIZE) {
        printf("IPv4 Packet had the wrong size: %d\n", ipv4.packet_size);
        return HAL_IPV4_BADPARAM;
    }

    /* TODO: make sure this is an IPv4 packet */
    ipv4.version = *(uint8_t *)(buf+offset) >> 4;
    if (ipv4.version != 4) {
        //printf("IPv4 Packet is not an actual IPv4 packet.\n");
        return HAL_IPV4_BADPARAM;
    }

    if (offset+20 > ipv4.packet_size) {
        printf("IPv4: Packet structural error.\n");
        return HAL_IPV4_BADPARAM;
    }

    ipv4.header_length = *(uint8_t *)(buf+offset) & 0x0f;
    if (ipv4.header_length < 5 ||
        ipv4.header_length * 4 > ipv4.packet_size) {
        printf("IPv4: Packet structural error.\n");
        return HAL_IPV4_BADPARAM;
    }
    ipv4.payload_length = ipv4.packet_size - ipv4.header_length*4;
    if (ipv4.payload_length > 0) {
        ipv4.payload_pointer = buf + (ipv4.header_length * 4);
    } else {
        ipv4.payload_pointer = NULL;
    }
    ipv4.dscp = (*(uint8_t *)(buf+offset+1) & 0xfc) >> 2;
    ipv4.ecn = *(uint8_t *)(buf+offset+1) & 0x03;
    ipv4.total_length = ntohs(*(uint16_t *)(buf+offset+2));
    if (ipv4.payload_length != ipv4.total_length && (flags & HAL_IPV4_PARSE_FAIL_ON_INVALID_LENGTH)) {
        printf("IPv4: Packet structural error.\n");
        return HAL_IPV4_BADPARAM;
    }
    ipv4.identification = ntohs(*(uint16_t *)(buf+offset+4));
    ipv4.flags = *(uint8_t *)(buf+offset+6) & 0x07;
    ipv4.fragment_offset = (ntohs(*(uint16_t *)(buf+offset+6)) & 0xf8) >> 3;
    ipv4.ttl = *(uint8_t *)(buf+offset+8);
    ipv4.protocol = *(uint8_t *)(buf+offset+9);
    ipv4.checksum = ntohs(*(uint16_t *)(buf+offset+10));
    ipv4.source_ip = ntohl(*(uint32_t *)(buf+offset+12));
    ipv4.destination_ip = ntohl(*(uint32_t *)(buf+offset+16));
    offset += 20;

    if (offset+1 > ipv4.packet_size) {
        printf("IPv4: Packet structural error.\n");
        return HAL_IPV4_BADPARAM;
    }

    for (i=0; ipv4.header_length>5 && i < HAL_IPV4_MAX_HEADERS; i++) {
        ipv4.number_of_options += 1;
        ipv4.header_options[i].copy_flag = *(uint8_t *)(buf+offset) & 0x01;
        ipv4.header_options[i].class_type = (*(uint8_t *)(buf+offset) & 0x06) >> 1;
        ipv4.header_options[i].number = (*(uint8_t *)(buf+offset) & 0xf8) >> 3;

        switch (ipv4.header_options[i].number) {
            case HAL_IPV4_OPT_END_OF_LIST:
                ipv4.header_options[i].length = 0;
                ipv4.header_options[i].data_pointer = NULL;
                goto out;
            /* simple options */
            case HAL_IPV4_OPT_NOP:
            case HAL_IPV4_OPT_COMMERCIAL_SECURITY:
            case HAL_IPV4_OPT_EXPERIMENTAL_MEASURE:
            case HAL_IPV4_OPT_EXP_FLOW_CONTROL:
            case HAL_IPV4_OPT_EXP_ACCESS_CONTROL:
            case HAL_IPV4_OPT_ENCODE:
            case HAL_IPV4_OPT_IMI_TRAFFIC_DESCRIPTOR:
            case HAL_IPV4_OPT_UNKNOWN_SIMPLE_1:
            case HAL_IPV4_OPT_DYNAMIC_PACKET_STATE:
            case HAL_IPV4_OPT_UPSTREAM_MULTICAST:
            case HAL_IPV4_OPT_QUICK_START:
            case HAL_IPV4_OPT_RFC3692_EXPERIMENTAL_1:
            case HAL_IPV4_OPT_RFC3692_EXPERIMENTAL_2:
            case HAL_IPV4_OPT_RFC3692_EXPERIMENTAL_3:
            case HAL_IPV4_OPT_RFC3692_EXPERIMENTAL_4:
                ipv4.header_options[i].data_pointer = NULL;
                ipv4.header_options[i].length = 0;
                offset += 1;
                break;
            default: /* It's assumed all other options include their own length */
                if (offset+1 > ipv4.header_length) {
                    //printf("IPv4: Bad Option.\n");
                    return HAL_IPV4_BADPARAM;
                }
                ipv4.header_options[i].length = *(uint8_t *)(buf+offset+1);
                offset += 1;
                if (offset+1+(ipv4.header_options[i].length-2) > ipv4.header_length) {
                    //printf("IPv4: Bad Option.\n");
                    return HAL_IPV4_BADPARAM;
                }
                ipv4.header_options[i].data_pointer = buf+offset+1;
                offset += 1 + (ipv4.header_options[i].length-2);
                break;
            /* end of default */
        }
        if (offset > ipv4.header_length*4) {
            printf("IPv4: Packet too small.\n");
            return HAL_IPV4_BADPARAM;
        }
        if (offset == ipv4.header_length*4) {
            goto out;
        }
    }

out:
            //ipv4_destination_ip_in_decimal(&ipv4);
            //ipv4_source_ip_in_decimal(&ipv4);
            //fprintf (stderr, "IPv4 packet data:\n");
            //fprintf (stderr, "  version=%d.\n", ipv4.version);
            //fprintf (stderr, "  header_length=%d.\n", ipv4.header_length);
            //fprintf (stderr, "  dscp=%d.\n", ipv4.dscp);
            //fprintf (stderr, "  ecn=%d.\n", ipv4.ecn);
            //fprintf (stderr, "  total_length=%d.\n", ipv4.total_length);
            //fprintf (stderr, "  identification=%d.\n", ipv4.identification);
            //fprintf (stderr, "  flags=%d.\n", ipv4.flags);
            //fprintf (stderr, "  fragment_offset=%d.\n", ipv4.fragment_offset);
            //fprintf (stderr, "  ttl=%d.\n", ipv4.ttl);
            //fprintf (stderr, "  protocol=%x (hex).\n", ipv4.protocol);
            //fprintf (stderr, "  checksum=%x (hex).\n", ipv4.checksum);
            //fprintf (stderr, "  source ip = %s.\n", ipv4.source_ip_decimal);
            //fprintf (stderr, "  destination ip = %s.\n", ipv4.destination_ip_decimal);
            //fprintf (stderr, "  number of options = %d.\n", ipv4.number_of_options);
            //fprintf (stderr, "  payload length = %d.\n", ipv4.total_length - ipv4.header_length*4);
            //fprintf (stderr, "  payload offset = %d.\n", ipv4.header_length*4);
            if (ipv4.number_of_options > 0) {
                //fprintf (stderr, "  IPv4 options were found.\n");
                fprintf (stderr, "  Number of options found: %d.\n", ipv4.number_of_options);
            }
            for (i=0;i<ipv4.number_of_options;i++) {
                //fprintf (stderr, "    copy flag = %0d\n", ipv4.header_options[i].copy_flag);
                //fprintf (stderr, "    class type = %0d\n", ipv4.header_options[i].class_type);
                //fprintf (stderr, "    option number = %0d\n", ipv4.header_options[i].number);
                //fprintf (stderr, "    length = %0d\n", ipv4.header_options[i].length);
            }

    // IPv4 Processer
    my_hal_ipv4_listener(&ipv4);
    tcp_parser(&ipv4);

    //printf("IPv4: Packet too small.\n");
    return HAL_IPV4_OK;
}

/*
int init(struct es_state *_state) {
    fprintf (stderr, "ipv4's init function was called.\n");
    hal_ethernet_register_listener register_function;
    int result;
    result = find_hal_symbol ("hal_ethernet", "register_listener", &(register_function));
    (*register_function) (my_eocene_ethernet_listener);
    return HAL_IPV4_OK;
}
*/
