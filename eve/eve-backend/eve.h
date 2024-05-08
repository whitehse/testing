#ifndef EVE_H
#define EVE_H

/*
#define EVE_OK               0 // successful result
#define EVE_FAIL            -1 // generic failure
#define EVE_NOMEM           -2 // memory shortage failure
#define EVE_BUFOVER         -3 // overflowed buffer
#define EVE_BADPARAM        -4 // invalid parameter supplied
#define EVE_BAD_DATA        -5 // Invalid Packet
*/

struct _ssh {
    struct assh_session_s *session;
    struct asshh_client_inter_session_s *inter;
    char *hostname;
    char *user;
    enum assh_userauth_methods_e *auth_methods;
    ev_io *socket_watcher_reader;
    int reader_running;
    ev_io *socket_watcher_writer;
    int writer_running;
};
typedef struct _ssh ssh_t;

struct _unix_domain {
};
typedef struct _unix_domain unix_domain_t;

int unix_domain_init(unix_domain_t *unix_domain);

struct _eve_curl {
};
typedef struct _eve_curl eve_curl_t;
 
int eve_curl_init(eve_curl_t *eve_curl);

#endif // EVE_H
