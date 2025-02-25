#ifndef NETCONF_H
#define NETCONF_H

/*
#define EVE_SSH_AGGREGATOR_OK               0 // successful result
#define EVE_SSH_AGGREGATOR_FAIL            -1 // generic failure
#define EVE_SSH_AGGREGATOR_NOMEM           -2 // memory shortage failure
#define EVE_SSH_AGGREGATOR_BUFOVER         -3 // overflowed buffer
#define EVE_SSH_AGGREGATOR_BADPARAM        -4 // invalid parameter supplied
#define EVE_SSH_AGGREGATOR_BAD_DATA        -5 // Invalid Packet
*/

enum netconf_server_type {
  NETCONF_SERVER_TYPE_NONE,
  NETCONF_SERVER_TYPE_CALIX,
  NETCONF_SERVER_TYPE_JUNIPER
};

enum calix_storage_type {
  CALIX_STORE_NONE,
  CALIX_STORE_ONT_DETAIL,
  CALIX_STORE_USER_LOGIN,
  CALIX_STORE_USER_LOGOUT,
  CALIX_STORE_ONT_US_SDBER,
};

struct tag_stack_tailq {
  enum xml_tag_type tag_type;
  enum xml_namespace namespace;
  char unknown_tag_text[51];
  TAILQ_ENTRY(tag_stack_tailq) tags;
};

TAILQ_HEAD(tag_stack_tailq_head, tag_stack_tailq);

struct ssh {
    struct assh_session_s *session;
    struct asshh_client_inter_session_s *inter;
    struct asshh_inter_subsystem_s *inter_subsystem;
    char *hostname;
    char *user;
    enum assh_userauth_methods_e *auth_methods;
    ev_io *socket_watcher_reader;
    ev_io *socket_watcher_writer;
    char *call_home_remote_address;
    struct assh_event_s *event;
    XML_Parser banner_parser;
    int banner_is_complete;
    int banner_ack_is_complete;
    XML_Parser hello_parser;
    int hello_end_tag_seen;
    int hello_is_complete;
    int hello_et_al_sent;
    XML_Parser message_parser;
    int incoming_message_is_complete;
    int outgoing_message_is_complete;
    //enum xml_tag_type message_tag_type;
    //enum xml_namespace xml_namespace;
    //enum xml_tag_stack tag_stack;
    //enum xml_tag_stack previous_tag_stack;
    struct tag_stack_tailq_head *tag_stack_head;
    uint32_t rpc_reply_message_id;
    char *xml_char_buffer;
    size_t xml_char_buffer_len;
    char *xmlns; 
    FILE *debug_file;
    enum netconf_server_type server_type;
    cbor_item_t* cbor_root;
    cbor_item_t* cbor_current_item;
    //uint32_t storage_type;
    enum calix_storage_type calix_storage_object;
    void *storage_struct;
    char *calix_remote_ip;
    //uint64_t juniper_storage_object;
};

struct juniper_banner {
//Juniper sends this (https://www.juniper.net/documentation/us/en/software/junos/netconf/topics/topic-map/netconf-call-home.html):
//MSG-ID: DEVICE-CONN-INFO\r\n
//MSG-VER: V1\r\n
//DEVICE-ID: <device-id>\r\n
//HOST-KEY: <public-host-key>\r\n
//HMAC:<HMAC(pub-SSH-host-key, <secret>)>\r\n
  char *msg_id;
  char *msg_ver;
  char *device_id;
  char *host_key;
  char *hmac;
};

struct calix_banner {
//<version>1</version>
//<identity>
//  <mac>00:02:5d:d9:21:47</mac>
//  <serial-number>071904926728</serial-number>
//  <model-name>E7 System</model-name>
//  <source-ip>192.168.35.13</source-ip>
//</identity>
  char *version;
  char *mac;
  char *serial_number;
  char *model_name;
  char *source_ip;
};

#endif // NETCONF_H
