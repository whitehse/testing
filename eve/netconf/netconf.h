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

struct ssh {
    struct assh_session_s *session;
    struct asshh_client_inter_session_s *inter;
    struct asshh_inter_subsystem_s *inter_subsystem;
    char *hostname;
    char *user;
    enum assh_userauth_methods_e *auth_methods;
    ev_io *socket_watcher_reader;
    ev_io *socket_watcher_writer;
    int banner_seen;
    int banner_written;
    char *call_home_remote_address;
    struct assh_event_s *event;
    XML_Parser banner_parser;
    XML_Parser message_parser;
};
typedef struct ssh ssh_t;

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
