#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <hal/ethernet.h>

//#define init hal_ethernet_LTX_init
//#define parse hal_ethernet_LTX_parse

hal_ethernet_listener *ethernet_listeners[HAL_ETH_MAX_LISTENERS];
int number_of_ethernet_listeners = 0;

int hal_eth_init(struct hal_state *_state) {
    return HAL_ETH_OK;
}

int register_listener(void *callback) {
    ethernet_listeners[number_of_ethernet_listeners] = callback;
    number_of_ethernet_listeners += 1;
}

/*void print_line(const char *line, unsigned n) {
    int i;
    int display_width=16;

    for(i=0;i<n;i++)
        fprintf(stderr, "%02x ", (unsigned char)line[i]);
    for(i=n;i<display_width;i++)
        fprintf(stderr, "   ");
    fprintf(stderr, " ");
    for(i=0;i<n;i++)
        if(isgraph((unsigned) line[i]))
            fprintf(stderr, "%c", line[i]);
        else
            fprintf(stderr, ".");
        fprintf(stderr, "\n");
}

void dump_buffer(const char* dump, unsigned n) {
    int i;

    if (dump == NULL) { return; }

    for (i=0;i<n;i+=16) {
        if(i + 16 <= n)
            print_line(dump+i, 16);
        else
            print_line(dump+i, n - i);
    }
}*/

char* destination_mac_in_dash_format (struct hal_ethernet* frame) {
    snprintf(frame->destination_mac_dash_format, 18,
        "%02x-%02x-%02x-%02x-%02x-%02x",
        (unsigned char)frame->destination_mac[0],
        (unsigned char)frame->destination_mac[1],
        (unsigned char)frame->destination_mac[2],
        (unsigned char)frame->destination_mac[3],
        (unsigned char)frame->destination_mac[4],
        (unsigned char)frame->destination_mac[5]);
    return frame->destination_mac_dash_format;
}

char* destination_mac_in_colon_format (struct hal_ethernet* frame) {
    snprintf(frame->destination_mac_colon_format, 18,
        "%02x:%02x:%02x:%02x:%02x:%02x",
        (unsigned char)frame->destination_mac[0],
        (unsigned char)frame->destination_mac[1],
        (unsigned char)frame->destination_mac[2],
        (unsigned char)frame->destination_mac[3],
        (unsigned char)frame->destination_mac[4],
        (unsigned char)frame->destination_mac[5]);
    return frame->destination_mac_colon_format;
}

char* destination_mac_in_cisco_format (struct hal_ethernet* frame) {
    snprintf(frame->destination_mac_cisco_format, 18,
        "%02x%02x.%02x%02x.%02x%02x",
        (unsigned char)frame->destination_mac[0],
        (unsigned char)frame->destination_mac[1],
        (unsigned char)frame->destination_mac[2],
        (unsigned char)frame->destination_mac[3],
        (unsigned char)frame->destination_mac[4],
        (unsigned char)frame->destination_mac[5]);
    return frame->destination_mac_cisco_format;
}

char* source_mac_in_dash_format (struct hal_ethernet* frame) {
    snprintf(frame->source_mac_dash_format, 18,
        "%02x-%02x-%02x-%02x-%02x-%02x",
        (unsigned char)frame->source_mac[0],
        (unsigned char)frame->source_mac[1],
        (unsigned char)frame->source_mac[2],
        (unsigned char)frame->source_mac[3],
        (unsigned char)frame->source_mac[4],
        (unsigned char)frame->source_mac[5]);
    return frame->source_mac_dash_format;
}

char* source_mac_in_colon_format (struct hal_ethernet* frame) {
    snprintf(frame->source_mac_colon_format, 18,
        "%02x:%02x:%02x:%02x:%02x:%02x",
        (unsigned char)frame->source_mac[0],
        (unsigned char)frame->source_mac[1],
        (unsigned char)frame->source_mac[2],
        (unsigned char)frame->source_mac[3],
        (unsigned char)frame->source_mac[4],
        (unsigned char)frame->source_mac[5]);
    return frame->source_mac_colon_format;
}

char* source_mac_in_cisco_format (struct hal_ethernet* frame) {
    snprintf(frame->source_mac_cisco_format, 18,
        "%02x%02x.%02x%02x.%02x%02x",
        (unsigned char)frame->source_mac[0],
        (unsigned char)frame->source_mac[1],
        (unsigned char)frame->source_mac[2],
        (unsigned char)frame->source_mac[3],
        (unsigned char)frame->source_mac[4],
        (unsigned char)frame->source_mac[5]);
    return frame->source_mac_cisco_format;
}

char* source_oui_in_dash_format (struct hal_ethernet* frame) {
    snprintf(frame->source_oui_dash_format, 18,
        "%02x-%02x-%02x",
        (unsigned char)frame->source_mac[0],
        (unsigned char)frame->source_mac[1],
        (unsigned char)frame->source_mac[2]);
    return frame->source_oui_dash_format;
}

char* source_oui_in_colon_format (struct hal_ethernet* frame) {
    snprintf(frame->source_oui_colon_format, 18,
        "%02x:%02x:%02x",
        (unsigned char)frame->source_mac[0],
        (unsigned char)frame->source_mac[1],
        (unsigned char)frame->source_mac[2]);
    return frame->source_oui_colon_format;
}

char* destination_oui_in_dash_format (struct hal_ethernet* frame) {
    snprintf(frame->destination_oui_dash_format, 18,
        "%02x-%02x-%02x",
        (unsigned char)frame->destination_mac[0],
        (unsigned char)frame->destination_mac[1],
        (unsigned char)frame->destination_mac[2]);
    return frame->destination_oui_dash_format;
}

char* destination_oui_in_colon_format (struct hal_ethernet* frame) {
    snprintf(frame->destination_oui_colon_format, 18,
        "%02x:%02x:%02x",
        (unsigned char)frame->destination_mac[0],
        (unsigned char)frame->destination_mac[1],
        (unsigned char)frame->destination_mac[2]);
    return frame->destination_oui_colon_format;
}

char* snap_oui_in_dash_format (struct hal_ethernet* frame) {
    snprintf(frame->snap_oui_dash_format, 18,
        "%02x-%02x-%02x",
        (unsigned char)frame->snap_oui[0],
        (unsigned char)frame->snap_oui[1],
        (unsigned char)frame->snap_oui[2]);
    return frame->snap_oui_dash_format;
}

char* snap_oui_in_colon_format (struct hal_ethernet* frame) {
    snprintf(frame->snap_oui_colon_format, 18,
        "%02x:%02x:%02x",
        (unsigned char)frame->snap_oui[0],
        (unsigned char)frame->snap_oui[1],
        (unsigned char)frame->snap_oui[2]);
    return frame->snap_oui_colon_format;
}

//int hal_eth_parse(const char* buf, unsigned n, int capture_type) {
int hal_eth_parse(char** ethernet_ptr, const char* buf, unsigned n) {
//    if (n < HAL_ETH_MIN_SNAPLEN || n > HAL_ETH_MAX_SNAPLEN) {
//        return HAL_ETH_BAD_FRAME;
//    }

    if (*ethernet_ptr == NULL) {
        printf("ethernet is null\n");
	*ethernet_ptr = malloc(10*sizeof(char));
    }

    if (buf == NULL) {
        return HAL_ETH_BAD_FRAME;
    }

    struct hal_ethernet frame;

    int i;
    uint16_t possible_ethertype = 0;
    int offset=0;
    int priority;
    int canonical_format;
    int vlan;
    uint32_t label;
    int cos;
    int stack_bit;
    int ttl;

    frame.frame_size = n;
    //frame.capture_type = capture_type;
    frame.destination_mac = buf;
    //printf("Destination MAC: %s\n", destination_mac_in_colon_format(&frame));
    frame.source_mac = buf + 6;
    frame.payload_length = 42;
    frame.done = 0;

    offset = 12;

    possible_ethertype = ntohs(*(uint16_t *) (buf+offset));

    for (i=0; i<32 && !frame.done; i++) {
        switch (possible_ethertype) {
            case HAL_ETH_ETHER_IPV4:
                frame.payload_ethertype = HAL_ETH_ETHER_IPV4;
                frame.payload_pointer = buf+offset+2;
                frame.payload_length = (n-(offset+2));
                //frame.payload_length = ntohs(*(uint16_t *)(n-(offset+2));
		//ipv4.total_length = ntohs(*(uint16_t *)(buf+offset+2));
                frame.done = 1;
                break;
            case HAL_ETH_ETHER_ARP:
                frame.payload_ethertype = HAL_ETH_ETHER_ARP;
                frame.payload_pointer = buf+offset+2;
                frame.payload_length = (n-(offset+2));
                frame.done = 1;
                break;
            case HAL_ETH_ETHER_IS_IS:
                printf("IS_IS Frame\n");
                return HAL_ETH_BAD_FRAME;
                break;
            case HAL_ETH_ETHER_REVERSE_ARP:
                frame.payload_ethertype = HAL_ETH_ETHER_REVERSE_ARP;
                frame.payload_pointer = buf+offset+2;
                frame.payload_length = (n-(offset+2));
                frame.done = 1;
                break;
            case HAL_ETH_ETHER_VLAN_TAG_8100:
                if (offset+4 > n) {
                    printf("Bad VLAN Tag\n");
                    return HAL_ETH_BAD_FRAME;
                }
                frame.contents = frame.contents | HAL_ETH_CONTAINS_VLAN_TAGS;
                priority = *(uint8_t *)(buf + offset+2) >> 5;
                canonical_format = *(uint8_t *)(buf + offset+2) & 0x10 >> 4;
                vlan = ntohs(*(uint16_t *)(buf + offset+2)) & 0x0fff;
                frame.vlan_tags[frame.number_of_vlan_tags].priority = priority;
                frame.vlan_tags[frame.number_of_vlan_tags].canonical_format = canonical_format;
                frame.vlan_tags[frame.number_of_vlan_tags].vlan = vlan;
                frame.vlan_tags[frame.number_of_vlan_tags].tag_type = HAL_ETH_ETHER_VLAN_TAG_8100;
                frame.number_of_vlan_tags += 1;
                if (frame.number_of_vlan_tags > HAL_ETH_MAX_VLAN_TAGS) {
                    printf("Too many VLAN tags\n");
                    return HAL_ETH_BAD_FRAME;
                }
                offset += 4;
                possible_ethertype = ntohs(*(uint16_t *) (buf+offset));
                break;
            case HAL_ETH_ETHER_IPV6:
                //printf("IPv6 Packet. Can't process yet.\n");
                return HAL_ETH_BAD_FRAME;
                break;
            case HAL_ETH_ETHER_TCP_IP_COMPRESSION:
                printf("TCP Compression. Can't process yet.\n");
                return HAL_ETH_BAD_FRAME;
                break;
            case HAL_ETH_ETHER_IP_AUTONOMOUS_SYSTEMS:
                printf("Autonomous Systems Packet. Can't process yet.\n");
                return HAL_ETH_BAD_FRAME;
                break;
            case HAL_ETH_ETHER_SECURE_DATA:
                printf("Secure Data Packet. Can't process yet.\n");
                return HAL_ETH_BAD_FRAME;
                break;
            case HAL_ETH_ETHER_LACP:
                printf("LACP Packet. Can't process yet.\n");
                return HAL_ETH_BAD_FRAME;
                break;
            case HAL_ETH_ETHER_PPP:
                printf("PPP Packet. Can't process yet.\n");
                return HAL_ETH_BAD_FRAME;
                break;
            case HAL_ETH_ETHER_VLAN_TAG_88A8:
                if (offset+4 > n) {
                    printf("Bad VLAN Tag. Can't process yet.\n");
                    return HAL_ETH_BAD_FRAME;
                }
                frame.contents = frame.contents | HAL_ETH_CONTAINS_VLAN_TAGS;
                priority = *(uint8_t *)(buf + offset+2) >> 5;
                canonical_format = *(uint8_t *)(buf + offset+2) & 0x10 >> 4;
                vlan = ntohs(*(uint16_t *)(buf + offset+2)) & 0x0fff;
                frame.vlan_tags[frame.number_of_vlan_tags].priority = priority;
                frame.vlan_tags[frame.number_of_vlan_tags].canonical_format = canonical_format;
                frame.vlan_tags[frame.number_of_vlan_tags].vlan = vlan;
                frame.vlan_tags[frame.number_of_vlan_tags].tag_type = HAL_ETH_ETHER_VLAN_TAG_88A8;
                frame.number_of_vlan_tags += 1;
                if (frame.number_of_vlan_tags > HAL_ETH_MAX_VLAN_TAGS) {
                    printf("Too many VLAN tags. Can't process yet.\n");
                    return HAL_ETH_BAD_FRAME;
                }
                offset += 4;
                possible_ethertype = ntohs(*(uint16_t *) (buf+offset));
                break;
            case HAL_ETH_ETHER_MPLS:
                if (offset+6 > n) {
                    printf("Bad MPLS tag. Can't process yet.\n");
                    return HAL_ETH_BAD_FRAME;
                }
                frame.contents = frame.contents | HAL_ETH_CONTAINS_MPLS_TAGS;
                offset += 2;
                do {
                    if (offset+4 > n) {
                        printf("Bad MPLS tag. Can't process yet.\n");
                        return HAL_ETH_BAD_FRAME;
                    }
                    label = (ntohs(*(uint16_t *)(buf+offset)) << 4) +
                        (((*(uint8_t*)(buf+offset+2)) & 0xf0) >> 4);
                    cos = ((*(uint8_t*)(buf+offset+2)) & 0x0e) >> 1;
                    stack_bit = (*(uint8_t*)(buf+offset+2)) & 0x01;
                    ttl = (*(uint8_t*)(buf+offset+3));
                    frame.mpls_tags[frame.number_of_mpls_tags].label = label;
                    frame.mpls_tags[frame.number_of_mpls_tags].cos = cos;
                    frame.mpls_tags[frame.number_of_mpls_tags].stack_bit = stack_bit;
                    frame.mpls_tags[frame.number_of_mpls_tags].ttl = ttl;
                    frame.number_of_mpls_tags += 1;
                    if (frame.number_of_mpls_tags > HAL_ETH_MAX_MPLS_TAGS) {
                        printf("Too many MPLS tag. Can't process yet.\n");
                        return HAL_ETH_BAD_FRAME;
                    }
                    offset += 4;
                } while (stack_bit == 0);
                frame.payload_pointer = buf+offset;
                frame.payload_length = n-offset;
                frame.payload_ethertype = HAL_ETH_ETHER_UNKNOWN;
                frame.done = 1;
                break;
            case HAL_ETH_ETHER_MPLS_UPSTREAM_LABEL:
                return HAL_ETH_BAD_FRAME;
                break;
            case HAL_ETH_ETHER_PPPOE_DISCOVERY:
                return HAL_ETH_BAD_FRAME;
                break;
            case HAL_ETH_ETHER_PPPOE_SESSION:
                printf("PPPoE Session. Can't process yet.\n");
                return HAL_ETH_BAD_FRAME;
                break;
            case HAL_ETH_ETHER_802_DOT_1X:
                printf("802.1x. Can't process yet.\n");
                return HAL_ETH_BAD_FRAME;
                break;
            case HAL_ETH_ETHER_VLAN_TAG_9100:
                if (offset+4 > n) {
                    printf("Bad VLAN Frame.\n");
                    return HAL_ETH_BAD_FRAME;
                }
                frame.contents = frame.contents | HAL_ETH_CONTAINS_VLAN_TAGS;
                priority = *(uint8_t *)(buf + offset+2) >> 5;
                canonical_format = *(uint8_t *)(buf + offset+2) & 0x10 >> 4;
                vlan = ntohs(*(uint16_t *)(buf + offset+2)) & 0x0fff;
                frame.vlan_tags[frame.number_of_vlan_tags].priority = priority;
                frame.vlan_tags[frame.number_of_vlan_tags].canonical_format = canonical_format;
                frame.vlan_tags[frame.number_of_vlan_tags].vlan = vlan;
                frame.vlan_tags[frame.number_of_vlan_tags].tag_type = HAL_ETH_ETHER_VLAN_TAG_9100;
                frame.number_of_vlan_tags += 1;
                if (frame.number_of_vlan_tags > HAL_ETH_MAX_VLAN_TAGS) {
                    printf("Too many VLAN tags. Can't process yet.\n");
                    return HAL_ETH_BAD_FRAME;
                }
                offset += 4;
                possible_ethertype = ntohs(*(uint16_t *) (buf+offset));
                break;
            case HAL_ETH_ETHER_VLAN_TAG_9200:
                if (offset+4 > n) {
                    printf("Bad VLAN tag.\n");
                    return HAL_ETH_BAD_FRAME;
                }
                frame.contents = frame.contents | HAL_ETH_CONTAINS_VLAN_TAGS;
                priority = *(uint8_t *)(buf + offset+2) >> 5;
                canonical_format = *(uint8_t *)(buf + offset+2) & 0x10 >> 4;
                vlan = ntohs(*(uint16_t *)(buf + offset+2)) & 0x0fff;
                frame.vlan_tags[frame.number_of_vlan_tags].priority = priority;
                frame.vlan_tags[frame.number_of_vlan_tags].canonical_format = canonical_format;
                frame.vlan_tags[frame.number_of_vlan_tags].vlan = vlan;
                frame.vlan_tags[frame.number_of_vlan_tags].tag_type = HAL_ETH_ETHER_VLAN_TAG_9200;
                frame.number_of_vlan_tags += 1;
                if (frame.number_of_vlan_tags > HAL_ETH_MAX_VLAN_TAGS) {
                    printf("Too many VLAN tags. Can't process yet.\n");
                    return HAL_ETH_BAD_FRAME;
                }
                offset += 4;
                possible_ethertype = ntohs(*(uint16_t *) (buf+offset));
                break;
            case HAL_ETH_ETHER_VLAN_TAG_9300:
                if (offset+4 > n) {
                    printf("Bad VLAN Tag. Can't process yet.\n");
                    return HAL_ETH_BAD_FRAME;
                }
                frame.contents = frame.contents | HAL_ETH_CONTAINS_VLAN_TAGS;
                priority = *(uint8_t *)(buf + offset+2) >> 5;
                canonical_format = *(uint8_t *)(buf + offset+2) & 0x10 >> 4;
                vlan = ntohs(*(uint16_t *)(buf + offset+2)) & 0x0fff;
                frame.vlan_tags[frame.number_of_vlan_tags].priority = priority;
                frame.vlan_tags[frame.number_of_vlan_tags].canonical_format = canonical_format;
                frame.vlan_tags[frame.number_of_vlan_tags].vlan = vlan;
                frame.vlan_tags[frame.number_of_vlan_tags].tag_type = HAL_ETH_ETHER_VLAN_TAG_9300;
                frame.number_of_vlan_tags += 1;
                if (frame.number_of_vlan_tags > HAL_ETH_MAX_VLAN_TAGS) {
                    printf("Too many VLAN tags. Can't process yet.\n");
                    return HAL_ETH_BAD_FRAME;
                }
                offset += 4;
                possible_ethertype = ntohs(*(uint16_t *) (buf+offset));
                break;
            case HAL_ETH_ETHER_RESERVED:
                printf("Reserved Ethernet Frame. Can't process yet.\n");
                return HAL_ETH_BAD_FRAME;
                break;
            default:
                /* Other ethertype, or unrecognized format */
                if (possible_ethertype > 1500) {
                    printf("Other ethertype (ethertype > 1500). Can't process yet.\n");
                    return HAL_ETH_BAD_FRAME;
                }

                /* This is an 802.3 frame */
                frame.contents = frame.contents | HAL_ETH_CONTAINS_802_DOT_3_LENGTH;
                frame.dot_3_length = possible_ethertype;
                offset += 2;

                if (offset+4 > n) {
                    printf("Bad Frame.\n");
                    return HAL_ETH_BAD_FRAME;
                }

                possible_ethertype = ntohs(*(uint16_t *) (buf+offset));

                /* Check for LLC */
                uint8_t llc_destination_sap = possible_ethertype;
                uint8_t llc_source_sap      = ntohs(*(uint8_t *) (buf+offset+1));

                /* IPX Raw 802.3 frame? */
                if (llc_destination_sap == HAL_ETH_SAP_GLOBAL_LSAP 
                    && llc_source_sap == HAL_ETH_SAP_GLOBAL_LSAP) {
                    frame.contents = frame.contents | HAL_ETH_CONTAINS_IPX_RAW_802_DOT_3;
                    frame.payload_ethertype = HAL_ETH_ETHER_RESERVED;
                    frame.payload_pointer = offset;
                    frame.payload_length = (n-offset);
                    frame.done = 1;
                    goto out;
                }

                /* Does have LLC */
                frame.contents = frame.contents | HAL_ETH_CONTAINS_LLC_HEADER;
                frame.llc_destination_sap = llc_destination_sap;
                frame.llc_source_sap      = llc_source_sap;

                /* Is this an unnumbered (U-Format/1 byte) PDU? */
                if ( (*(uint8_t *)(buf+offset+2)) & 3) {
                    frame.llc_control = *(uint8_t *)(buf + offset+2);
                    offset += 3;
                } else {
                    frame.contents = frame.contents | HAL_ETH_CONTAINS_2_BYTE_LLC_CONTROL;
                    frame.llc_control = ntohs(*(uint16_t *)(buf+offset+2));
                    offset += 4;
                }

                if (offset+5 > n) {
                    printf("Bad Frame.\n");
                    return HAL_ETH_BAD_FRAME;
                }
                /* Is SNAP service requested? */
                if ((llc_destination_sap == HAL_ETH_SAP_SNAP_AA
                     || llc_destination_sap == HAL_ETH_SAP_SNAP_AB)
                   && (llc_source_sap == HAL_ETH_SAP_SNAP_AA
                     || llc_source_sap == HAL_ETH_SAP_SNAP_AB)) {

                    printf("Bad ethernet frame where analyzing SNAP headers.\n");
                    return HAL_ETH_BAD_FRAME;
/* Need to debug this
                    frame.contents = frame.contents | HAL_ETH_CONTAINS_SNAP_HEADER;
                    memcpy (frame.snap_oui, buf+offset, 3);
                    frame.snap_sub_protocol = ntohs(*(uint16_t *)(buf+offset+3));
                    offset += 3;
*/
                } else {
                    switch (llc_destination_sap) {
                        case HAL_ETH_SAP_IEEE_802_DOT_1_STP:
                            frame.payload_ethertype = HAL_ETH_ETHER_802_DOT_1_STP;
                            frame.payload_pointer = buf+offset;
                            frame.payload_length = (n-(offset));
                            frame.done = 1;
                            goto out;
                            break;
                        default: /* unregocnized SAP */
                            printf("Unrecognized SAP. Can't process yet.\n");
                            return HAL_ETH_BAD_FRAME;
                            break;
                    }
                } /* No recognizable Ethertype */

                /* RFC 1042 ethertype SNAP? */
                if (frame.llc_control == 3 && frame.snap_oui[0] == 0 &&
                   frame.snap_oui[1] == 0 && frame.snap_oui[2]) {
                    possible_ethertype = ntohs(*(uint16_t *) (buf+offset));
                } else {
                    printf("Something went wrong while processing SNAP header. Can't process yet.\n");
                    return HAL_ETH_BAD_FRAME;
                }
            /* End of Default Case */
        }
    }

out:

    // Further processing
    //for (i=0; i<number_of_ethernet_listeners; i++) {
    //    hal_ethernet_listener listener_cb = ethernet_listeners[i];
    //    listener_cb (frame);
    //}

    // IPv4 Processer
    my_hal_ethernet_listener(&frame);

    //printf("The frame was processed correctly!\n");
    return HAL_ETH_OK;
}
