#ifndef EVE_NETCONF_H
#define EVE_NETCONF_H

/*
#define EVE_NETCONF_OK               0 // successful result
#define EVE_NETCONF_FAIL            -1 // generic failure
#define EVE_NETCONF_NOMEM           -2 // memory shortage failure
#define EVE_NETCONF_BUFOVER         -3 // overflowed buffer
#define EVE_NETCONF_BADPARAM        -4 // invalid parameter supplied
#define EVE_NETCONF_BAD_DATA        -5 // Invalid Packet
*/

struct ssh {
    struct assh_session_s *session;
    struct asshh_client_inter_session_s *inter;
    char* hostname;
    char* user;
    enum assh_userauth_methods_e *auth_methods;
};
typedef struct ssh ssh_t;

#endif // EVE_NETCONF_H
