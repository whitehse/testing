/*

  libassh - asynchronous ssh2 client/server library.

  Copyright (C) 2013-2020 Alexandre Becoulet <alexandre.becoulet@free.fr>

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301 USA

*/

/**
   @file
   @short Session structure and related functions

   This header file provides declaration of the @ref assh_session_s
   structure and related functions, used to create and manage @em ssh2
   @hl sessions.
*/

#ifndef ASSH_SESSION_H_
#define ASSH_SESSION_H_

#include "assh_transport.h"
#include "assh_service.h"
#include "assh_queue.h"
#include "assh_packet.h"
#include "assh_algo.h"

/** @see assh_kex_filter_t */
#define ASSH_KEX_FILTER_FCN(n)                          \
  assh_bool_t (n)(struct assh_session_s *s,             \
                  const struct assh_algo_s *algo,       \
                  const struct assh_algo_name_s *name,  \
                  assh_bool_t out)

/** This is a per session algorithm filtering function.

    @tt 1 must be returned in order to make the algorithm available
    for the session @hl key-exchange. This can not be used to prevent
    registered signature algorithms from being used during the user
    authentication process.

    The result of this function must not vary between calls for a
    given algorithm, unless the @ref assh_session_algo_filter function
    has been called successfully.

    The @tt out parameter specifies the direction and is relevant for
    cipher, mac and compression algorithms.

    @xsee {suppalgos} */
typedef ASSH_KEX_FILTER_FCN(assh_kex_filter_t);

#if CONFIG_ASSH_IDENT_SIZE < 8 || CONFIG_ASSH_IDENT_SIZE > 255
# error CONFIG_ASSH_IDENT_SIZE out of range
#endif

/** @internalmembers @This is the @em ssh2 @hl session state
    structure.

    A @hl session instance is associated to an @ref assh_context_s
    object which holds resources shared between multiple sessions.

    It is @b not related to @hl{interactive sessions} which are
    part of the @hl{connection protocol}. */
struct assh_session_s
{
  /** User private pointer */
  ASSH_PV void *user_pv;

  /** Pointer to associated main context. */
  ASSH_PV struct assh_context_s *ctx;

  /** Key exchange algorithm. This pointer is setup when the @ref
      assh_kex_got_init function select a new key exchange algorithm. */
  ASSH_PV const struct assh_algo_kex_s *kex_algo;
  /** Key exchange private context used during key exchange only. */
  ASSH_PV void *kex_pv;

  /** Pointer to the last key exechange packet sent by client. Valid
      during key exechange. */
  ASSH_PV struct assh_packet_s *kex_init_local;
  /** Pointer to the last key exechange packet sent by client. Valid
      during key exechange. */
  ASSH_PV struct assh_packet_s *kex_init_remote;
  /** remote side prefered kex and host signature algorithms */
  ASSH_PV const struct assh_algo_s *kex_preferred[2];

  /** per session algorithm filter */
  ASSH_PV assh_kex_filter_t *kex_filter;

  /** amount of data transfered since last kex */
  ASSH_PV uint32_t kex_bytes;

  /** kex re-exchange threshold */
  ASSH_PV uint32_t kex_max_bytes;

  /** Host keys signature algorithm */
  ASSH_PV const struct assh_algo_sign_s *host_sign_algo;

  /** Current service. */
  ASSH_PV const struct assh_service_s *srv;
  /** Current service private data. */
  ASSH_PV void *srv_pv;

  /** Host key sent by the server. The key released when the
      @ref ASSH_EVENT_KEX_DONE event is acknowledged.  */
  ASSH_PV struct assh_key_s *kex_host_key;

  /** Queue of ssh output packets. Packets in this queue will be
      enciphered and sent. */
  ASSH_PV struct assh_queue_s out_queue;
  /** Alternate queue of ssh output packets, used to store services
      packets during a key exchange. */
  ASSH_PV struct assh_queue_s alt_queue;

  /** Pointer to output keys and algorithms in current use. */
  ASSH_PV struct assh_kex_keys_s *cur_keys_out;
  /** Pointer to next output keys and algorithms on SSH_MSG_NEWKEYS transmitted. */
  ASSH_PV struct assh_kex_keys_s *new_keys_out;

  /** Current input ssh stream packet. This packet is currently being
      read from the input ssh stream and has not yet been deciphered. */
  ASSH_PV struct assh_packet_s *stream_in_pck;

  /** Current ssh input packet. This packet is the last deciphered
      packets and is waiting for dispatch and processing. */
  ASSH_PV struct assh_packet_s *in_pck;

  /** Pointer to input keys and algorithms in current use. */
  ASSH_PV struct assh_kex_keys_s *cur_keys_in;
  /** Pointer to next input keys and algorithms on SSH_MSG_NEWKEYS received. */
  ASSH_PV struct assh_kex_keys_s *new_keys_in;

  /** last error reported to @ref assh_session_error. This will be
      reported as an @ref ASSH_EVENT_SESSION_ERROR event. */
  ASSH_PV assh_status_t last_err;

  /** Current date as reported by the last IO request. */
  ASSH_PV assh_time_t time;

  /** The session will terminate with the @ref ASSH_ERR_TIMEOUT error
      if this field contains a value less than the @ref time field. It
      is updated by the transport layer. */
  ASSH_PV assh_time_t tr_deadline;

  /** The running service may update this field and check when this
      field contains a value less than the @ref time field. When not
      used, it must be set to 0 so that it is excluded from the
      computation of the next protocol timeout reported to the
      application. */
  ASSH_PV assh_time_t srv_deadline;

  /** The key-exchange process will be stated again when this field
      contains a value less then the @ref time field and the transport
      state is @ref ASSH_TR_SERVICE. */
  ASSH_PV assh_time_t rekex_deadline;

  /** Size of valid data in the @ref stream_in_pck packet */
  ASSH_PV size_t stream_in_size:32;
  /** Size of already sent data of the top packet in the @ref out_queue queue. */
  ASSH_PV size_t stream_out_size:32;
  /** Amount of data currently in the 2 output queues. */
  ASSH_PV size_t queue_out_size:32;

  /** Input packet sequence number */
  ASSH_PV uint32_t in_seq;
  /** Output packet sequence number */
  ASSH_PV uint32_t out_seq;

  /** Current input ssh stream header buffer. */
  union  {
    ASSH_PV uint8_t data[ASSH_MAX_BLOCK_SIZE];
    ASSH_PV struct assh_packet_head_s head;
  } stream_in_stub;

  /** Session id is first "exchange hash" H */
  ASSH_PV uint8_t session_id[ASSH_MAX_HASH_SIZE];
  /** Session id length */
  ASSH_PV size_t session_id_len:8;

  /** Copy of the ident string sent by the remote host. */
  ASSH_PV uint8_t ident_str[CONFIG_ASSH_IDENT_SIZE];
  /** Size of the ident string sent by the remote host. */
  ASSH_PV size_t ident_len:8;

#ifdef CONFIG_ASSH_CLIENT
  /** Index of the next service to request in the context services array. */
  ASSH_PV size_t srv_index:5;
#endif

  /** user authentication success packet has been handled by the
      transport layer */
  ASSH_PV assh_bool_t tr_user_auth_done:1;
  /** user authentication success */
  ASSH_PV assh_bool_t user_auth_done:1;

  /** initial key exchange done. */
  ASSH_PV assh_bool_t kex_done:1;

  /** Currrent output ssh stream generator state. */
  ASSH_PV enum assh_stream_out_state_e stream_out_st:3;

  /** Current input ssh stream parser state. */
  ASSH_PV enum assh_stream_in_state_e stream_in_st:3;

  /** Current state of the transport layer. */
  ASSH_PV enum assh_transport_state_e tr_st:4;

  /** Current state of service execution. */
  ASSH_PV enum assh_service_state_e srv_st:3;

#ifndef NDEBUG
  ASSH_PV assh_bool_t event_done:1;
#endif
};

/** The @ref ASSH_EVENT_SESSION_ERROR event is reported when an error
    occurs. Because not all errors are fatal, the event may be
    reported multiple times during a single session.

    @see #ASSH_STATUS @see #ASSH_SEVERITY */
struct assh_event_session_error_s
{
  /** The error code reported by the library. (ro) */
  ASSH_EV_CONST assh_status_t code;
};

/** @This contains all session related event structures. */
union assh_event_session_u
{
  struct assh_event_session_error_s error;
};

/** @This sets the user private pointer of the session.
    @see assh_session_get_pv */
void assh_session_set_pv(struct assh_session_s *ctx,
                         void *private);

/** @This retrieves the user private pointer of the session.
    @see assh_session_set_pv */
void * assh_session_get_pv(const struct assh_session_s *ctx);

/** @This initializes an user allocated session
    instance.  When a stable ABI is needed, the @ref
    assh_session_create function muse be used instead.

    @see assh_session_cleanup
*/
ASSH_ABI_UNSAFE ASSH_WARN_UNUSED_RESULT assh_status_t
assh_session_init(struct assh_context_s *c,
		  struct assh_session_s *s);

/** @This allocates and initializes an @ref assh_session_s instance.

    @see assh_session_release
*/
ASSH_WARN_UNUSED_RESULT assh_status_t
assh_session_create(struct assh_context_s *c,
		    struct assh_session_s **s);

/** @This releases the resources associated with an user allocated
    @ref assh_session_s instance. */
ASSH_ABI_UNSAFE void
assh_session_cleanup(struct assh_session_s *s);

/** @This releases an @ref assh_session_s instance created by the
    @ref assh_session_create function as well as associated
    resources. */
void assh_session_release(struct assh_session_s *s);

/** @internal @This changes the session state according to the
    provided error code and associated severity level.

    @This returns the original error code but the error
    severity level may be increased. This function is responsible for
    sending the session close message to the remote hsot.

    @This is called from the @ref assh_event_get, @ref
    assh_event_done function. It is also called from other functions
    of the public API which can modify the session state.

    @see assh_status_e @see assh_severity_e
*/
ASSH_PV void
assh_session_error(struct assh_session_s *s, assh_status_t err);

/** @This schedules the end of the session and sends an
    SSH_MSG_DISCONNECT message to the remote host. The @ref
    assh_event_get function must still be called until no more events
    are available. */
assh_status_t
assh_session_disconnect(struct assh_session_s *s,
                        enum assh_ssh_disconnect_e reason,
                        const char *desc);

/** @This returns the current session safety factor which depends on
    algorithms and keys involved in the last @hl{key-exchange}
    process. The safety factor may change during the session
    lifetime.  @see assh_algo_register_va */
assh_safety_t assh_session_safety(const struct assh_session_s *s);

/** @This setups a per session algorithm filter for the @hl
    key-exchange. The @tt filter parameter may be @tt NULL to disable
    filtering. It will fail if a key exchange is currently
    running. @xsee{suppalgos} */
ASSH_WARN_UNUSED_RESULT assh_status_t
assh_session_algo_filter(struct assh_session_s *s,
                         assh_kex_filter_t *filter);

/** @This returns the next protocol deadline. In order for the
    library to handle protocol timeouts properly, the process must not
    wait forever on a blocking io operation. The @ref assh_event_get
    function must be called again when the deadline is reached.

    @see assh_session_delay
*/
assh_time_t
assh_session_deadline(const struct assh_session_s *s);

/** @This returns the delay between the next protocol deadline and the
    current time. The current time must be passed to the function in
    second units. If the next deadline is in the past, the function
    returns 0.

    @see assh_session_deadline
*/
assh_time_t
assh_session_delay(const struct assh_session_s *s, assh_time_t time);

/** @This returns true when the @ref assh_event_get function will not
    report more events. */
assh_bool_t
assh_session_closed(const struct assh_session_s *s);

/** @This marks the user as authenticated. This is usually called by
    the user authentication services. On rare cases when one of these
    services is not used, it may be called directly by the application. */
void
assh_session_userauth_done(struct assh_session_s *s);

/** @This returns the context associated to the given session. */
struct assh_context_s *
assh_session_context(struct assh_session_s *s);

#endif

