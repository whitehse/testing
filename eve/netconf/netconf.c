#include <assh/assh_session.h>
#include <assh/assh_context.h>
#include <assh/assh_service.h>
#include <assh/assh_userauth_client.h>
#include <assh/assh_compress.h>
#include <assh/assh_connection.h>
#include <assh/helper_key.h>
#include <assh/helper_interactive.h>
#include <assh/helper_client.h>
#include <assh/assh_kex.h>
#include <assh/helper_io.h>
#include <assh/assh_event.h>
#include <assh/assh_algo.h>

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <ev.h>
#include <eio.h>
#include <netconf.h>

#define ERROR(...) do { fprintf(stderr, __VA_ARGS__); exit(1); } while (0)

/*
 * Events:
 *   ASSH_EVENT_READ                       - event is reported in order to gather incoming ssh stream data from the remote host.
 *   ASSH_EVENT_WRITE                      - reported when some ssh stream data is available for sending to the remote host.
 *   ASSH_EVENT_DISCONNECT                 - reported when the remote host transmitted an SSH_MSG_DISCONNECT message.
 *   ASSH_EVENT_DEBUG                      - reported when the remote host transmitted an SSH_MSG_DEBUG message.
 *   ASSH_EVENT_SESSION_ERROR              - reported when an error occurs.
 *   ASSH_EVENT_KEX_HOSTKEY_LOOKUP         - event is returned when a client needs to lookup a server host key in the local database.
 *   ASSH_EVENT_KEX_DONE                   - event is returned when a kex exchange has completed.
 *   ASSH_EVENT_SERVICE_START              - event is reported when a service has started.
 *   ASSH_EVENT_USERAUTH_CLIENT_USER       - event is reported when the client-side user authentication service is running and the service needs to provide a user name to the server.
 *   ASSH_EVENT_USERAUTH_CLIENT_METHODS    - event is reported when the client-side user authentication service is running, before every authentication attempt. The methods field indicates the authentication methods that are accepted by the server. One of these methods must be selected by setting the select field.
 *   ASSH_EVENT_USERAUTH_CLIENT_BANNER     - event is reported when the client-side user authentication service is running and a banner message is received.
 *   ASSH_EVENT_USERAUTH_CLIENT_PWCHANGE   - event is reported when the client-side user authentication service is running and a password change request message is received.
 *   ASSH_EVENT_USERAUTH_CLIENT_KEYBOARD   - event is reported when the keyboard interactive authentication has been selected and the server sent a SSH_MSG_USERAUTH_INFO_REQUEST message.
 *   ASSH_EVENT_USERAUTH_CLIENT_SUCCESS    - 
 *   ASSH_EVENT_USERAUTH_CLIENT_SIGN       - event is reported when the client-side user authentication service is running and a public key has been provided for public key authentication.
 *   ASSH_EVENT_USERAUTH_SERVER_METHODS    - event is reported when the server-side user authentication service is running and some authentication methods must be selected.
 *   ASSH_EVENT_USERAUTH_SERVER_NONE       - event is reported when the server-side user authentication service is running and the client has selected the none method.
 *   ASSH_EVENT_USERAUTH_SERVER_USERKEY    - event is reported when the server-side user authentication service is running and the client has selected the user public key method.
 *   ASSH_EVENT_USERAUTH_SERVER_PASSWORD   - event is reported when the server-side user authentication service is running and the client has selected the password method.
 *   ASSH_EVENT_USERAUTH_SERVER_KBINFO     - event is reported when the server-side user authentication service is running and the client has selected the keyboard interactive method.
 *   ASSH_EVENT_USERAUTH_SERVER_KBRESPONSE - event is reported when the server-side user authentication service is running and the client has replied to a previous SSH_MSG_USERAUTH_INFO_REQUEST message by sending a SSH_MSG_USERAUTH_INFO_RESPONSE message.
 *   ASSH_EVENT_USERAUTH_SERVER_HOSTBASED  - event is reported when the server-side user authentication service is running and the client has selected the hostbased method.
 *   ASSH_EVENT_USERAUTH_SERVER_SUCCESS    - event is reported when an user authentication request is successful. The method field indicates which method has been used successfully.
 *   ASSH_EVENT_REQUEST                    - event is reported when an user authentication request is successful. The method field indicates which method has been used successfully.
 *   ASSH_EVENT_REQUEST_ABORT              - event is reported when a channel is closing or the connection is ending and some associated requests have been postponed.
 *   ASSH_EVENT_REQUEST_SUCCESS            - event is reported after a successful call to the assh_request function when the want_reply parameter was set and the remote host replied with a success message.
 *   ASSH_EVENT_REQUEST_FAILURE            - event is reported after a successful call to the assh_channel_open function when the channel open is rejected by the remote host.
 *   ASSH_EVENT_CHANNEL_OPEN               - event is reported after a successful call to the assh_channel_open function when the channel open is rejected by the remote host.
 *   ASSH_EVENT_CHANNEL_CONFIRMATION       - event is reported after a successful call to the assh_channel_open function when the channel open is accepted by the remote host.
 *   ASSH_EVENT_CHANNEL_FAILURE            - event is reported after a successful call to the assh_channel_open function when the channel open is rejected by the remote host.
 *   ASSH_EVENT_CHANNEL_DATA               - event is reported when the assh_service_connection service is running and some incoming channel data are available.
 *   ASSH_EVENT_CHANNEL_WINDOW             - event is reported when the assh_service_connection service is running and an SSH_MSG_CHANNEL_WINDOW_ADJUST message has been received.
 *   ASSH_EVENT_CHANNEL_EOF                - event is reported when the assh_service_connection service is running and the remote host has sent the SSH_MSG_CHANNEL_EOF message.
 *   ASSH_EVENT_CHANNEL_CLOSE              - event is reported for open channels when the channel is in ASSH_CHANNEL_ST_CLOSING state and all data and requests associated with the channel have been reported using appropriate events.
 *   ASSH_EVENT_CHANNEL_ABORT              - event is reported when the connection is ending and some channel open have been postponed.
*/

ev_io stdin_watcher;
ev_io stdout_watcher;
ev_timer timeout_watcher;

static void timeout_cb (EV_P_ ev_timer *w, int revents) {
  fprintf (stderr, "timeout\n");
}

static void socket_cb (struct ev_loop *loop, ev_io *w, int revents) {
  time_t t = time(NULL);
  ssh_t *ssh = w->data;

  while (1)
    {
      struct assh_event_s event;

      if (!assh_event_get(ssh->session, &event, t))
        return;

      switch (event.id)
	{

	case ASSH_EVENT_READ:
	case ASSH_EVENT_WRITE:

	  asshh_fd_event(ssh->session, &event, w->fd);
	  break;

	case ASSH_EVENT_SESSION_ERROR:
	  fprintf(stderr, "SSH error: %s\n",
		  assh_error_str(event.session.error.code));
	  assh_event_done(ssh->session, &event, ASSH_OK);
	  break;

        case ASSH_EVENT_KEX_HOSTKEY_LOOKUP:
          asshh_client_event_hk_lookup(ssh->session, stderr, stdin, ssh->hostname, &event);
          break;

        case ASSH_EVENT_USERAUTH_CLIENT_BANNER:
        case ASSH_EVENT_USERAUTH_CLIENT_USER:
        case ASSH_EVENT_USERAUTH_CLIENT_METHODS:
        case ASSH_EVENT_USERAUTH_CLIENT_PWCHANGE:
        case ASSH_EVENT_USERAUTH_CLIENT_KEYBOARD:
          asshh_client_event_auth(ssh->session, stderr, stdin, ssh->user, ssh->hostname,
             ssh->auth_methods, asshh_client_user_key_default, &event);

          struct assh_event_userauth_client_user_s *eu =
            &event.userauth_client.user;

          assh_buffer_strset(&eu->username, ssh->user);
          break;

        case ASSH_EVENT_SERVICE_START:
        case ASSH_EVENT_CHANNEL_CONFIRMATION:
        case ASSH_EVENT_CHANNEL_FAILURE:
        case ASSH_EVENT_REQUEST_SUCCESS:
        case ASSH_EVENT_REQUEST_FAILURE:
        case ASSH_EVENT_CHANNEL_CLOSE:
          asshh_client_event_inter_session(ssh->session, &event, ssh->inter);

	  if (ssh->inter->state == ASSH_CLIENT_INTER_ST_CLOSED)
	    assh_session_disconnect(ssh->session, SSH_DISCONNECT_BY_APPLICATION, NULL);
          break;

	case ASSH_EVENT_CHANNEL_DATA: {
          struct assh_event_channel_data_s *ec = &event.connection.channel_data;
          assh_status_t err = ASSH_OK;

          ssize_t r = write(1, ec->data.data, ec->data.size);
          if (r < 0)
            err = ASSH_ERR_IO;
          else
            ec->transferred = r;

          assh_event_done(ssh->session, &event, err);
          break;
	}

	default:
	  assh_event_done(ssh->session, &event, ASSH_OK);
	}
    }
}

int main(int argc, char **argv) {
  if (assh_deps_init())
    ERROR("initialization error\n");

  if (argc < 3)
    ERROR("usage: ./rexec host 'command'\n");

  const char *user = getenv("USER");
  if (user == NULL)
    ERROR("Unspecified user name\n");

  const char *hostname = argv[1];
  const char *command = argv[2];
  //const char *port = "22";
  const char *port = "830";

  struct ev_loop *loop = EV_DEFAULT;
  ssh_t ssh;

  struct addrinfo hints = {
    .ai_family = AF_UNSPEC,
    .ai_socktype = SOCK_STREAM,
  };

  int sock = -1;
  struct addrinfo *servinfo, *si;
  if (!getaddrinfo(hostname, port, &hints, &servinfo))
    {
      for (si = servinfo; si != NULL; si = si->ai_next)
        {
          sock = socket(si->ai_family, si->ai_socktype, si->ai_protocol);
          if (sock < 0)
            continue;

          if (connect(sock, si->ai_addr, si->ai_addrlen))
            {
              close(sock);
              sock = -1;
              continue;
            }

          break;
        }

      freeaddrinfo(servinfo);
    }

  if (sock < 0)
    ERROR("Unable to connect: %s\n", strerror(errno));

  signal(SIGPIPE, SIG_IGN);

  struct assh_context_s *context;

  if (assh_context_create(&context, ASSH_CLIENT,
                          NULL, NULL, NULL, NULL) ||
      assh_service_register_default(context) ||
      assh_algo_register_default(context, ASSH_SAFETY_WEAK))
    ERROR("Unable to create an assh context.\n");

  struct assh_session_s *session;

  if (assh_session_create(context, &session))
    ERROR("Unable to create an assh session.\n");

  enum assh_userauth_methods_e auth_methods =
    ASSH_USERAUTH_METHOD_PASSWORD |
    ASSH_USERAUTH_METHOD_PUBKEY |
    ASSH_USERAUTH_METHOD_KEYBOARD;

  struct asshh_client_inter_session_s inter;
  //asshh_client_init_inter_session(&inter, command, NULL);
  asshh_client_init_inter_session(&inter, NULL, NULL);

  //struct asshh_inter_subsystem_s sub;
  //asshh_inter_init_subsystem(&sub, "netconf");

  struct assh_event_s event;

  ssh.session = session;
  ssh.inter = &inter;
  ssh.hostname = hostname;
  ssh.user = user;
  ssh.auth_methods = &auth_methods;

  ev_io socket_watcher;
  ev_io_init (&socket_watcher, socket_cb, sock, EV_READ|EV_WRITE);
  socket_watcher.data = &ssh;
  ev_io_start (loop, &socket_watcher);

  ev_timer_init (&timeout_watcher, timeout_cb, 5.5, 1.);
  ev_timer_start (loop, &timeout_watcher);

  ev_run (loop, 0);

  abort();

  printf("Connection closed\n");

  assh_session_release(session);
  assh_context_release(context);

  return 0;
}


/*
<?xml version="1.0" encoding="UTF-8"?>
<hello xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
<capabilities>
<capability>urn:ietf:params:netconf:base:1.0</capability>
<capability>urn:ietf:params:netconf:base:1.1</capability>
<capability>urn:ietf:params:netconf:capability:writable-running:1.0</capability>
<capability>urn:ietf:params:netconf:capability:rollback-on-error:1.0</capability>
<capability>urn:ietf:params:netconf:capability:url:1.0?scheme=ftp,sftp,file</capability>
<capability>urn:ietf:params:netconf:capability:validate:1.0</capability>
<capability>urn:ietf:params:netconf:capability:validate:1.1</capability>
<capability>urn:ietf:params:netconf:capability:xpath:1.0</capability>
<capability>urn:ietf:params:netconf:capability:notification:1.0</capability>
<capability>urn:ietf:params:netconf:capability:interleave:1.0</capability>
<capability>urn:ietf:params:netconf:capability:with-defaults:1.0?basic-mode=trim&amp;also-supported=report-all-tagged,report-all</capability>
<capability>urn:ietf:params:netconf:capability:with-operational-defaults:1.0?basic-mode=trim&amp;also-supported=report-all-tagged,report-all</capability>
<capability>urn:ietf:params:netconf:capability:yang-library:1.0?revision=2019-01-04&amp;module-set-id=1732182466ce607796e3546a93b03eba</capability>
<capability>urn:ietf:params:netconf:capability:yang-library:1.1?revision=2019-01-04&amp;content-id=1732182466ce607796e3546a93b03eba</capability>
<capability>http://tail-f.com/ns/aaa/1.1?module=tailf-aaa&amp;revision=2013-03-07</capability>
<capability>http://tail-f.com/ns/common/query?module=tailf-common-query&amp;revision=2017-04-27</capability>
<capability>http://tail-f.com/ns/webui?module=tailf-webui&amp;revision=2013-03-07</capability>
<capability>http://tail-f.com/yang/acm?module=tailf-acm&amp;revision=2013-03-07</capability>
<capability>http://tail-f.com/yang/common?module=tailf-common&amp;revision=2021-09-21</capability>
<capability>http://tail-f.com/yang/common-monitoring?module=tailf-common-monitoring&amp;revision=2019-04-09</capability>
<capability>http://tail-f.com/yang/confd-monitoring?module=tailf-confd-monitoring&amp;revision=2019-10-30</capability>
<capability>http://tail-f.com/yang/netconf-monitoring?module=tailf-netconf-monitoring&amp;revision=2012-06-14</capability>
<capability>http://www.calix.com/ns/arc?module=arc&amp;revision=2022-06-29</capability>
<capability>http://www.calix.com/ns/erps-g8032-qos?module=erps-g8032-qos&amp;revision=2020-08-18</capability>
<capability>http://www.calix.com/ns/ethernet-std?module=ethernet-std&amp;revision=2022-05-30</capability>
<capability>http://www.calix.com/ns/exa/access-security?module=access-security&amp;revision=2021-08-20</capability>
<capability>http://www.calix.com/ns/exa/access-security-std?module=access-security-std&amp;revision=2022-03-31</capability>
<capability>http://www.calix.com/ns/exa/acl?module=acl&amp;revision=2022-03-30</capability>
<capability>http://www.calix.com/ns/exa/acl-router-igmp?module=acl-router-igmp&amp;revision=2021-01-08</capability>
<capability>http://www.calix.com/ns/exa/acl-std?module=acl-std&amp;revision=2022-05-10</capability>
<capability>http://www.calix.com/ns/exa/ae-management?module=ae-management&amp;revision=2022-02-17</capability>
<capability>http://www.calix.com/ns/exa/ae-management-std?module=ae-management-std&amp;revision=2019-04-18</capability>
<capability>http://www.calix.com/ns/exa/axos-netconf-server-base?module=axos-netconf-server-base&amp;revision=2020-09-10</capability>
<capability>http://www.calix.com/ns/exa/axos-netconf-server-std?module=axos-netconf-server-std&amp;revision=2021-01-12</capability>
<capability>http://www.calix.com/ns/exa/base?module=exa-base&amp;revision=2020-03-16&amp;features=shelf-config,ont-simulation,dos-protection,chassis-craft-port,backplane-mode</capability>
<capability>http://www.calix.com/ns/exa/bridge?module=bridge&amp;revision=2022-03-18</capability>
<capability>http://www.calix.com/ns/exa/bridge-std-eth?module=bridge-std-eth&amp;revision=2021-04-21</capability>
<capability>http://www.calix.com/ns/exa/bridge-std-lag?module=bridge-std-lag&amp;revision=2021-04-21</capability>
<capability>http://www.calix.com/ns/exa/bridge-std-ont?module=bridge-std-ont&amp;revision=2022-03-18</capability>
<capability>http://www.calix.com/ns/exa/card-profiles?module=card-profiles&amp;revision=2022-05-30</capability>
<capability>http://www.calix.com/ns/exa/copp?module=copp&amp;revision=2022-05-30</capability>
<capability>http://www.calix.com/ns/exa/copp-std?module=copp-std&amp;revision=2022-05-30</capability>
<capability>http://www.calix.com/ns/exa/crypto-types?module=crypto-types&amp;revision=2018-03-07</capability>
<capability>http://www.calix.com/ns/exa/diagnostics?module=diagnostics&amp;revision=2022-03-02</capability>
<capability>http://www.calix.com/ns/exa/diagnostics-std?module=diagnostics-std&amp;revision=2018-08-17</capability>
<capability>http://www.calix.com/ns/exa/discovery?module=discovery&amp;revision=2021-08-20</capability>
<capability>http://www.calix.com/ns/exa/dot1x-base?module=dot1x-base&amp;revision=2022-02-25</capability>
<capability>http://www.calix.com/ns/exa/dot1x-std-ethernet?module=dot1x-std-ethernet&amp;revision=2021-04-20</capability>
<capability>http://www.calix.com/ns/exa/dscp?module=dscp&amp;revision=2021-08-20</capability>
<capability>http://www.calix.com/ns/exa/dscp-copp?module=dscp-copp&amp;revision=2020-10-22</capability>
<capability>http://www.calix.com/ns/exa/dscp-lag-std?module=dscp-lag-std&amp;revision=2022-05-16</capability>
<capability>http://www.calix.com/ns/exa/dscp-std?module=dscp-std&amp;revision=2022-05-10</capability>
<capability>http://www.calix.com/ns/exa/equipment-ua?module=equipment-ua&amp;revision=2020-12-14</capability>
<capability>http://www.calix.com/ns/exa/erps?module=erps&amp;revision=2022-03-25</capability>
<capability>http://www.calix.com/ns/exa/erps-rstp?module=erps-rstp&amp;revision=2020-10-22</capability>
<capability>http://www.calix.com/ns/exa/erps-std-eth?module=erps-std-eth&amp;revision=2020-02-25</capability>
<capability>http://www.calix.com/ns/exa/erps-std-lag?module=erps-std-lag&amp;revision=2020-02-25</capability>
<capability>http://www.calix.com/ns/exa/ethernet-service-oam?module=ethernet-service-oam&amp;revision=2022-05-31</capability>
<capability>http://www.calix.com/ns/exa/ethernet-soam-std?module=ethernet-soam-std&amp;revision=2021-09-16</capability>
<capability>http://www.calix.com/ns/exa/exa-entity-base?module=exa-entity-base&amp;revision=2021-08-17</capability>
<capability>http://www.calix.com/ns/exa/ffp-base?module=ffp-base&amp;revision=2021-11-22</capability>
<capability>http://www.calix.com/ns/exa/ffp-std?module=ffp-std&amp;revision=2021-08-18</capability>
<capability>http://www.calix.com/ns/exa/g8032?module=g8032&amp;revision=2022-05-03</capability>
<capability>http://www.calix.com/ns/exa/g8032-std-eth?module=g8032-std-eth&amp;revision=2021-09-16</capability>
<capability>http://www.calix.com/ns/exa/g8032-std-lag?module=g8032-std-lag&amp;revision=2020-02-25</capability>
<capability>http://www.calix.com/ns/exa/gpon-dscp-std?module=gpon-dscp-std&amp;revision=2019-12-24</capability>
<capability>http://www.calix.com/ns/exa/gpon-interface-base?module=gpon-interface-base&amp;revision=2022-04-26</capability>
<capability>http://www.calix.com/ns/exa/gpon-interface-soam?module=gpon-interface-soam&amp;revision=2021-12-19</capability>
<capability>http://www.calix.com/ns/exa/gpon-interface-soam-std?module=gpon-interface-soam-std&amp;revision=2022-02-14</capability>
<capability>http://www.calix.com/ns/exa/gpon-interface-std?module=gpon-interface-std&amp;revision=2022-07-27</capability>
<capability>http://www.calix.com/ns/exa/gpon-ont-simulation-base?module=gpon-ont-simulation-base&amp;revision=2021-08-20</capability>
<capability>http://www.calix.com/ns/exa/host-management-std?module=host-management-std&amp;revision=2022-07-06</capability>
<capability>http://www.calix.com/ns/exa/igmp?module=igmp&amp;revision=2021-08-20</capability>
<capability>http://www.calix.com/ns/exa/igmp-ont?module=igmp-ont&amp;revision=2022-03-31</capability>
<capability>http://www.calix.com/ns/exa/igmp-std-eth?module=igmp-std-eth&amp;revision=2022-03-31</capability>
<capability>http://www.calix.com/ns/exa/igmp-std-lag?module=igmp-std-lag&amp;revision=2020-11-17</capability>
<capability>http://www.calix.com/ns/exa/igmp-std-ont?module=igmp-std-ont&amp;revision=2021-03-05</capability>
<capability>http://www.calix.com/ns/exa/ip-management?module=ip-management&amp;revision=2022-05-10</capability>
<capability>http://www.calix.com/ns/exa/ip-management-eth-std?module=ip-management-eth-std&amp;revision=2022-06-28</capability>
<capability>http://www.calix.com/ns/exa/ip-management-lag-std?module=ip-management-lag-std&amp;revision=2022-07-06</capability>
<capability>http://www.calix.com/ns/exa/ip-management-vlan-std?module=ip-management-vlan-std&amp;revision=2022-07-06</capability>
<capability>http://www.calix.com/ns/exa/ipdr?module=ipdr&amp;revision=2022-03-24</capability>
<capability>http://www.calix.com/ns/exa/ipfix-base?module=ipfix-base&amp;revision=2022-04-14</capability>
<capability>http://www.calix.com/ns/exa/ipfix-bgp-std?module=ipfix-bgp-std&amp;revision=2021-09-02</capability>
<capability>http://www.calix.com/ns/exa/ipfix-eth-std?module=ipfix-eth-std&amp;revision=2021-09-02</capability>
<capability>http://www.calix.com/ns/exa/ipfix-gpon-std?module=ipfix-gpon-std&amp;revision=2021-09-02</capability>
<capability>http://www.calix.com/ns/exa/ipfix-igmp-std?module=ipfix-igmp-std&amp;revision=2021-09-02</capability>
<capability>http://www.calix.com/ns/exa/ipfix-lag-std?module=ipfix-lag-std&amp;revision=2021-09-02</capability>
<capability>http://www.calix.com/ns/exa/ipfix-routing-std?module=ipfix-routing-std&amp;revision=2021-09-01</capability>
<capability>http://www.calix.com/ns/exa/ipfix-std?module=ipfix-std&amp;revision=2021-06-09</capability>
<capability>http://www.calix.com/ns/exa/ipfix-vlan-std?module=ipfix-vlan-std&amp;revision=2021-09-02</capability>
<capability>http://www.calix.com/ns/exa/lacp?module=lacp&amp;revision=2022-02-25</capability>
<capability>http://www.calix.com/ns/exa/lacp-lag-base?module=lacp-lag-base&amp;revision=2019-12-10</capability>
<capability>http://www.calix.com/ns/exa/lacp-std-eth?module=lacp-std-eth&amp;revision=2017-08-02</capability>
<capability>http://www.calix.com/ns/exa/lacp-std-lag?module=lacp-std-lag&amp;revision=2020-11-17</capability>
<capability>http://www.calix.com/ns/exa/layer2-control-protocol?module=layer2-control-protocol&amp;revision=2021-08-20</capability>
<capability>http://www.calix.com/ns/exa/layer2-control-protocol-std?module=layer2-control-protocol-std&amp;revision=2020-11-17</capability>
<capability>http://www.calix.com/ns/exa/layer2-service-protocols?module=layer2-service-protocols&amp;revision=2022-04-08</capability>
<capability>http://www.calix.com/ns/exa/layer2-service-protocols-std?module=layer2-service-protocols-std&amp;revision=2022-03-31</capability>
<capability>http://www.calix.com/ns/exa/lldp?module=lldp&amp;revision=2022-03-24</capability>
<capability>http://www.calix.com/ns/exa/logging-std?module=logging-std&amp;revision=2021-07-20</capability>
<capability>http://www.calix.com/ns/exa/mka-base?module=mka-base&amp;revision=2022-02-25</capability>
<capability>http://www.calix.com/ns/exa/mka-std?module=mka-std&amp;revision=2020-11-17</capability>
<capability>http://www.calix.com/ns/exa/ont-upgrade?module=ont-upgrade&amp;revision=2022-05-16</capability>
<capability>http://www.calix.com/ns/exa/pcp-map?module=pcp-map&amp;revision=2022-02-26</capability>
<capability>http://www.calix.com/ns/exa/pcp-map-std?module=pcp-map-std&amp;revision=2022-02-26</capability>
<capability>http://www.calix.com/ns/exa/platform-std?module=platform-std&amp;revision=2021-02-24</capability>
<capability>http://www.calix.com/ns/exa/pppoe?module=pppoe&amp;revision=2022-02-25</capability>
<capability>http://www.calix.com/ns/exa/pppoeia?module=pppoeia&amp;revision=2022-07-04</capability>
<capability>http://www.calix.com/ns/exa/pppoeia-std?module=pppoeia-std&amp;revision=2022-03-31</capability>
<capability>http://www.calix.com/ns/exa/pwe-services?module=pwe-services&amp;revision=2021-06-28</capability>
<capability>http://www.calix.com/ns/exa/rbac?module=rbac&amp;revision=2022-02-25</capability>
<capability>http://www.calix.com/ns/exa/router-bfd?module=router-bfd&amp;revision=2022-07-24</capability>
<capability>http://www.calix.com/ns/exa/shelf-slot-ua?module=shelf-slot-ua&amp;revision=2021-11-17</capability>
<capability>http://www.calix.com/ns/exa/vca?module=vca&amp;revision=2021-06-27</capability>
<capability>http://www.calix.com/ns/exa/voip-services?module=voip-services&amp;revision=2022-07-22</capability>
<capability>http://www.calix.com/ns/exa/voip-services-std?module=voip-services-std&amp;revision=2022-04-18</capability>
<capability>http://www.calix.com/ns/iana-timezones?module=iana-timezones&amp;revision=2019-10-11</capability>
<capability>http://www.calix.com/ns/interface?module=interface&amp;revision=2021-12-06</capability>
<capability>http://www.calix.com/ns/ipfix-vrf?module=ipfix-vrf&amp;revision=2021-10-07</capability>
<capability>http://www.calix.com/ns/lag?module=lag&amp;revision=2020-09-08</capability>
<capability>http://www.calix.com/ns/lag-soam-std?module=lag-soam-std&amp;revision=2021-09-16</capability>
<capability>http://www.calix.com/ns/lag-std?module=lag-std&amp;revision=2022-05-30</capability>
<capability>http://www.calix.com/ns/lldp-std?module=lldp-std&amp;revision=2022-02-17</capability>
<capability>http://www.calix.com/ns/qos-std?module=qos-std&amp;revision=2020-09-08</capability>
<capability>http://www.calix.com/ns/remote-mirror?module=remote-mirror&amp;revision=2021-09-02</capability>
<capability>http://www.calix.com/ns/remote-mirror-std?module=remote-mirror-std&amp;revision=2021-07-08</capability>
<capability>http://www.calix.com/ns/router-bgp?module=router-bgp&amp;revision=2022-05-18</capability>
<capability>http://www.calix.com/ns/router-igmp?module=router-igmp&amp;revision=2021-12-06</capability>
<capability>http://www.calix.com/ns/router-isis?module=router-isis&amp;revision=2022-07-29</capability>
<capability>http://www.calix.com/ns/router-ospf?module=router-ospf&amp;revision=2022-07-20</capability>
<capability>http://www.calix.com/ns/router-ospf-lag-std?module=router-ospf-lag-std&amp;revision=2022-07-06</capability>
<capability>http://www.calix.com/ns/router-ospf-std?module=router-ospf-std&amp;revision=2022-07-06</capability>
<capability>http://www.calix.com/ns/router-ospf-vrf?module=router-ospf-vrf&amp;revision=2022-07-06</capability>
<capability>http://www.calix.com/ns/router-pim?module=router-pim&amp;revision=2022-07-27</capability>
<capability>http://www.calix.com/ns/router-plcy?module=router-plcy&amp;revision=2021-09-02</capability>
<capability>http://www.calix.com/ns/router-rip?module=router-rip&amp;revision=2021-08-20</capability>
<capability>http://www.calix.com/ns/rstp?module=rstp&amp;revision=2022-02-25</capability>
<capability>http://www.calix.com/ns/rstp-std-eth?module=rstp-std-eth&amp;revision=2020-11-17</capability>
<capability>http://www.calix.com/ns/rstp-std-lag?module=rstp-std-lag&amp;revision=2020-07-08</capability>
<capability>http://www.calix.com/ns/sensor-bcm-switch?module=sensor-bcm-switch&amp;revision=2021-08-06</capability>
<capability>http://www.calix.com/ns/system-settings?module=system-settings&amp;revision=2022-04-28</capability>
<capability>http://www.calix.com/ns/vlan-interface-std?module=vlan-interface-std&amp;revision=2022-05-13</capability>
<capability>http://www.calix.com/ns/vrf-routing?module=vrf-routing&amp;revision=2022-06-16</capability>
<capability>http://www.calix.com/ns/wifi-interface?module=wifi-interface&amp;revision=2022-02-25</capability>
<capability>http://www.calix.com/ns/wifi-interface-std?module=wifi-interface-std&amp;revision=2019-12-09</capability>
<capability>http://www.verizon.com/ns/verizon-ipfix?module=verizon-ipfix&amp;revision=2021-06-09</capability>
<capability>urn:dummy-common?module=tailf-common-monitoring-ann</capability>
<capability>urn:dummy-ietf-interfaces-ann?module=ietf-interfaces-ann</capability>
<capability>urn:dummy-monitoring?module=ietf-netconf-monitoring-ann</capability>
<capability>urn:dummy-verizon-ipfix-ann?module=verizon-ipfix-ann</capability>
<capability>urn:dummy2?module=tailf-confd-monitoring-ann</capability>
<capability>urn:dummy4?module=ietf-netconf-acm-ann</capability>
<capability>urn:dummy5?module=ietf-netconf-notifications-ann</capability>
<capability>urn:dummy6?module=ietf-restconf-ann</capability>
<capability>urn:dummy7?module=ietf-restconf-monitoring-ann</capability>
<capability>urn:dummy8?module=ietf-yang-library-ann</capability>
<capability>urn:ietf:params:xml:ns:netconf:base:1.0?module=ietf-netconf&amp;revision=2011-06-01&amp;features=writable-running,rollback-on-error,validate,xpath,url</capability>
<capability>urn:ietf:params:xml:ns:yang:ietf-inet-types?module=ietf-inet-types&amp;revision=2013-07-15</capability>
<capability>urn:ietf:params:xml:ns:yang:ietf-interfaces?module=ietf-interfaces&amp;revision=2018-02-20&amp;features=if-mib</capability>
<capability>urn:ietf:params:xml:ns:yang:ietf-ipfix-psamp?module=ietf-ipfix-psamp&amp;revision=2022-03-01&amp;features=udpTransport,timeoutCache,sctpTransport,psampSampUniProb,psampSampTimeBased,psampSampRandOutOfN,psampSampCountBased,psampFilterMatch,psampFilterHash,permanentCache,naturalCache,meter,immediateCache,fileWriter,fileReader</capability>
<capability>urn:ietf:params:xml:ns:yang:ietf-netconf-acm?module=ietf-netconf-acm&amp;revision=2012-02-22</capability>
<capability>urn:ietf:params:xml:ns:yang:ietf-netconf-monitoring?module=ietf-netconf-monitoring&amp;revision=2010-10-04</capability>
<capability>urn:ietf:params:xml:ns:yang:ietf-netconf-notifications?module=ietf-netconf-notifications&amp;revision=2012-02-06</capability>
<capability>urn:ietf:params:xml:ns:yang:ietf-netconf-server-2015-11-12?module=ietf-netconf-server&amp;revision=2015-11-12&amp;features=ssh-call-home</capability>
<capability>urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults?module=ietf-netconf-with-defaults&amp;revision=2011-06-01</capability>
<capability>urn:ietf:params:xml:ns:yang:ietf-network-instance?module=ietf-network-instance&amp;revision=2022-03-21</capability>
<capability>urn:ietf:params:xml:ns:yang:ietf-restconf-monitoring?module=ietf-restconf-monitoring&amp;revision=2016-08-15</capability>
<capability>urn:ietf:params:xml:ns:yang:ietf-yang-smiv2?module=ietf-yang-smiv2&amp;revision=2011-11-25</capability>
<capability>urn:ietf:params:xml:ns:yang:ietf-yang-types?module=ietf-yang-types&amp;revision=2013-07-15</capability>
<capability>urn:ietf:params:xml:ns:netconf:notification:1.0?module=notifications&amp;revision=2008-07-14</capability>
</capabilities>
<session-id>2523</session-id></hello>]]>]]>
*/
