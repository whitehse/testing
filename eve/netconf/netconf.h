#ifndef EVE_SSH_AGGREGATOR_H
#define EVE_SSH_AGGREGATOR_H

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
    int reader_running;
    ev_io *socket_watcher_writer;
    int writer_running;
    int banner_seen;
    int banner_written;
    char *call_home_remote_address;
    struct assh_event_s *event;
};
typedef struct ssh ssh_t;

#endif // EVE_SSH_AGGREGATOR_H
