/*
 * Copyright (C) 2014, 2023 Dan White
 * BSD two-clause License
 *
 */

#ifndef HAL_ETHERNET_H
#define HAL_ETHERNET_H

/* Return types. But only use these internally! */
/* Use, for example, HAL_OK, when returning from public
 * functions. */
#define HAL_ETH_OK               0 /* successful result */
#define HAL_ETH_FAIL            -1 /* generic failure */
#define HAL_ETH_NOMEM           -2 /* memory shortage failure */
#define HAL_ETH_BUFOVER         -3 /* overflowed buffer */
#define HAL_ETH_BADPARAM        -4 /* invalid parameter supplied */
#define HAL_ETH_BAD_FRAME       -5 /* Invalid Packet */

/*#define HAL_ETH_MAX_COMPONENTS 128*/  /* Maximum number of headers and components that
                                           the library will parse */

#define HAL_ETH_MIN_SNAPLEN 60
#define HAL_ETH_MAX_SNAPLEN 32764

/* Capture types */
#define HAL_ETH_CAP_TYPE_MAC  0  /* Frame begins with destination MAC */
#define HAL_ETH_CAP_TYPE_PCAP 1  /* Ditto. PCAP doesn't include framing bits */

/* Frame types */
#define HAL_ETH_FRAME_ETHERNET_II 1  /* Ethertype */
#define HAL_ETH_FRAME_802_DOT_3   2  /* Length + 802.2 LLC */
/* 802.3 requires an LLC (802.2) header, which in turn optionally supports a
 * SNAP header */

#define HAL_ETH_FRAME_STP_WITHOUT_SNAP 0x4242

#define HAL_ETH_MAX_VLAN_TAGS 4
#define HAL_ETH_MAX_MPLS_TAGS 4

/* Contents */
#define HAL_ETH_CONTAINS_802_DOT_3_LENGTH   1
#define HAL_ETH_CONTAINS_IPX_RAW_802_DOT_3  2
#define HAL_ETH_CONTAINS_LLC_HEADER         4
#define HAL_ETH_CONTAINS_2_BYTE_LLC_CONTROL 8
#define HAL_ETH_CONTAINS_SNAP_HEADER        16
#define HAL_ETH_CONTAINS_VLAN_TAGS          32
#define HAL_ETH_CONTAINS_MPLS_TAGS          64

/* Well known Ethertypes */
/* Others can be registered via TODO */
#define HAL_ETH_ETHER_UNKNOWN               0x0000
#define HAL_ETH_ETHER_IPV4                  0x0800
#define HAL_ETH_ETHER_ARP                   0x0806
#define HAL_ETH_ETHER_802_DOT_1_STP         0x4242
#define HAL_ETH_ETHER_IS_IS                 0x8000
#define HAL_ETH_ETHER_REVERSE_ARP           0x8035
#define HAL_ETH_ETHER_VLAN_TAG_8100         0x8100
#define HAL_ETH_ETHER_IPV6                  0x86dd
#define HAL_ETH_ETHER_TCP_IP_COMPRESSION    0x876b
#define HAL_ETH_ETHER_IP_AUTONOMOUS_SYSTEMS 0x876c
#define HAL_ETH_ETHER_SECURE_DATA           0x876d
#define HAL_ETH_ETHER_LACP                  0x8809
#define HAL_ETH_ETHER_PPP                   0x880b
#define HAL_ETH_ETHER_VLAN_TAG_88A8         0x88a8
#define HAL_ETH_ETHER_MPLS                  0x8847
#define HAL_ETH_ETHER_MPLS_UPSTREAM_LABEL   0x8848
#define HAL_ETH_ETHER_PPPOE_DISCOVERY       0x8863
#define HAL_ETH_ETHER_PPPOE_SESSION         0x8864
#define HAL_ETH_ETHER_802_DOT_1X            0x888e
#define HAL_ETH_ETHER_LLDP                  0x88cc
#define HAL_ETH_ETHER_VLAN_TAG_9100         0x9100
#define HAL_ETH_ETHER_VLAN_TAG_9200         0x9200
#define HAL_ETH_ETHER_VLAN_TAG_9300         0x9300
#define HAL_ETH_ETHER_RESERVED              0xffff

/* Well known Service Access Points (SAPs) */
/* Others can be registered via TODO */
#define HAL_ETH_SAP_NULL                                 0x00
#define HAL_ETH_SAP_INDIVIDUAL_LLC_SUBLAYER              0x02
#define HAL_ETH_SAP_GROUP_LLC_SUBLAYER                   0x03
#define HAL_ETH_SAP_IBM_SNA_INDIVIDUAL_PATH_CONTROL      0x04
#define HAL_ETH_SAP_IBM_SNA_GROUP_PATH_CONTROL           0x05
#define HAL_ETH_SAP_ARPANET_INTERNET_PROTOCOL            0x06
#define HAL_ETH_SAP_SNA                                  0x08
#define HAL_ETH_SAP_SNA2                                 0x0c
#define HAL_ETH_SAP_PROWAY_NETWORK_MANAGEMENT            0x0e
#define HAL_ETH_SAP_TEXAS_INSTRUMENTS                    0x18
#define HAL_ETH_SAP_IEEE_802_DOT_1_STP                   0x42
#define HAL_ETH_SAP_EIA_RS_511_MESSAGE                   0x4e
#define HAL_ETH_SAP_ISO_8208                             0x7e
#define HAL_ETH_SAP_XEROX_NETWORK_SYSTEMS                0x80
#define HAL_ETH_SAP_NESTAR                               0x86
#define HAL_ETH_SAP_PROWAY_ACTIVE_STATION                0x8e
#define HAL_ETH_SAP_ARPANET_ARP                          0x98
#define HAL_ETH_SAP_BANYAN_VINES                         0xbc
#define HAL_ETH_SAP_SNAP_AA                              0xaa
#define HAL_ETH_SAP_SNAP_AB                              0xab
#define HAL_ETH_SAP_NOVELL_NETWARE                       0xe0
#define HAL_ETH_SAP_IBM_NETBIOS                          0xf0
#define HAL_ETH_SAP_IBM_INDIVIDUAL_LAN_MANAGEMENT        0xf4
#define HAL_ETH_SAP_IBM_GROUP_LAN_MANAGEMENT             0xf5
#define HAL_ETH_SAP_IBM_REMOTE_PROGRAM_LOAD              0xf8
#define HAL_ETH_SAP_UNGERMANN_BASS                       0xfa
#define HAL_ETH_SAP_ISO_NETWORK_LAYER_PROTOCOL           0xfe
#define HAL_ETH_SAP_GLOBAL_LSAP                          0xff

struct hal_eth_vlan_tag {
    int tag_type;
    int priority;
    int canonical_format;
    int vlan;
};

struct hal_eth_mpls_tag {
    uint32_t label;
    int cos;
    int stack_bit;
    int ttl;
};

struct hal_ethernet {
    int frame_size;
    //int capture_type;
    //char source_mac[7];     /* All MACs/OUIs are in network byte order */
    char *source_mac;         /* All MACs/OUIs are in network byte order */
    char source_mac_dash_format[18];
    char source_mac_colon_format[18];
    char source_mac_cisco_format[18];
    //char destination_mac[7];
    char *destination_mac;
    char destination_mac_dash_format[18];
    char destination_mac_colon_format[18];
    char destination_mac_cisco_format[18];
    char source_oui_dash_format[9];
    char source_oui_colon_format[9];
    char destination_oui_dash_format[9];
    char destination_oui_colon_format[9];
    int contents;
    int dot_3_length;
    uint16_t llc_destination_sap; /* Host byte order */
    uint16_t llc_source_sap;      /* Host byte order */
    uint16_t llc_control;         /* Host byte order. >=0, <=65535 */
    char snap_oui[3];             /* Network byte order */
    char snap_oui_dash_format[9];
    char snap_oui_colon_format[9];
    uint16_t snap_sub_protocol;   /* Host byte order */
    int number_of_vlan_tags;
    struct hal_eth_vlan_tag vlan_tags[HAL_ETH_MAX_VLAN_TAGS];
    int number_of_mpls_tags;
    struct hal_eth_mpls_tag mpls_tags[HAL_ETH_MAX_MPLS_TAGS];
    uint16_t payload_ethertype;
    char* payload_pointer;
    int payload_length;
    int done;
};

typedef int (*hal_ethernet_listener)(struct hal_ethernet *frame);
typedef int (*hal_ethernet_register_listener)(void *callback);

#define HAL_ETH_MAX_LISTENERS          128

/*void print_line(const char *buf, unsigned n);
void dump_buffer(const char* buf, unsigned n);*/
char* destination_mac_in_dash_format (struct hal_ethernet* frame);
char* destination_mac_in_colon_format (struct hal_ethernet* frame);
char* destination_mac_in_cisco_format (struct hal_ethernet* frame);
char* source_mac_in_dash_format (struct hal_ethernet* frame);
char* source_mac_in_colon_format (struct hal_ethernet* frame);
char* source_mac_in_cisco_format (struct hal_ethernet* frame);
char* source_oui_in_dash_format (struct hal_ethernet* frame);
char* source_oui_in_colon_format (struct hal_ethernet* frame);
char* destination_oui_in_dash_format (struct hal_ethernet* frame);
char* destination_oui_in_colon_format (struct hal_ethernet* frame);
char* snap_oui_in_dash_format (struct hal_ethernet* frame);
char* snap_oui_in_colon_format (struct hal_ethernet* frame);

#endif // HAL_ETHERNET_H
