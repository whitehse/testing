//#define CONFIG_ASSH_DEBUG
//#define CONFIG_ASSH_DEBUG_EVENT
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
//#include <linux/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/eventfd.h>
#include <liburing.h>
#include <fcntl.h>

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

#include <ev.h>
#include <cbor.h>
#include <cjson/cJSON.h>
#include <netconf.h>

#define ERROR(...) do { fprintf(stderr, __VA_ARGS__); exit(1); } while (0)
#define MSG_OUT stdout /* Send info to stdout, change to stderr if you want */
#define DPRINT(x...) printf(x)

#define BUFFER_SIZE 1024

struct ev_loop *loop;
int netconf_listener_fd;

cJSON* cbor_to_cjson(cbor_item_t* item) {
  switch (cbor_typeof(item)) {
    case CBOR_TYPE_UINT:
      return cJSON_CreateNumber(cbor_get_int(item));
    case CBOR_TYPE_NEGINT:
      return cJSON_CreateNumber(-1 - cbor_get_int(item));
    case CBOR_TYPE_BYTESTRING:
      // cJSON only handles null-terminated string -- binary data would have to
      // be escaped
      return cJSON_CreateString("Unsupported CBOR item: Bytestring");
    case CBOR_TYPE_STRING:
      if (cbor_string_is_definite(item)) {
        // cJSON only handles null-terminated string
        char* null_terminated_string = malloc(cbor_string_length(item) + 1);
        memcpy(null_terminated_string, cbor_string_handle(item),
               cbor_string_length(item));
        null_terminated_string[cbor_string_length(item)] = 0;
        cJSON* result = cJSON_CreateString(null_terminated_string);
        free(null_terminated_string);
        return result;
      }
      return cJSON_CreateString("Unsupported CBOR item: Chunked string");
    case CBOR_TYPE_ARRAY: {
      cJSON* result = cJSON_CreateArray();
      for (size_t i = 0; i < cbor_array_size(item); i++) {
        cJSON_AddItemToArray(result, cbor_to_cjson(cbor_move(cbor_array_get(item, i))));
      }
      return result;
    }
    case CBOR_TYPE_MAP: {
      cJSON* result = cJSON_CreateObject();
      for (size_t i = 0; i < cbor_map_size(item); i++) {
        char* key = malloc(128);
        snprintf(key, 128, "Surrogate key %zu", i);
        // JSON only support string keys
        if (cbor_isa_string(cbor_map_handle(item)[i].key) &&
            cbor_string_is_definite(cbor_map_handle(item)[i].key)) {
          size_t key_length = cbor_string_length(cbor_map_handle(item)[i].key);
          if (key_length > 127) key_length = 127;
          // Null-terminated madness
          memcpy(key, cbor_string_handle(cbor_map_handle(item)[i].key),
                 key_length);
          key[key_length] = 0;
        }

        cJSON_AddItemToObject(result, key,
                              cbor_to_cjson(cbor_map_handle(item)[i].value));
        free(key);
      }
      return result;
    }
    case CBOR_TYPE_TAG:
      return cJSON_CreateString("Unsupported CBOR item: Tag");
    case CBOR_TYPE_FLOAT_CTRL:
      if (cbor_float_ctrl_is_ctrl(item)) {
        if (cbor_is_bool(item)) return cJSON_CreateBool(cbor_get_bool(item));
        if (cbor_is_null(item)) return cJSON_CreateNull();
        return cJSON_CreateString("Unsupported CBOR item: Control value");
      }
      return cJSON_CreateNumber(cbor_float_get_float(item));
  }

  return cJSON_CreateNull();
}

void hexDump(char *desc, void *addr, int len)
{
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    // Output description if given.
    if (desc != NULL)
        printf ("%s:\n", desc);

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf("  %s\n", buff);

            // Output the offset.
            printf("  %04x ", i);
        }

        // Now the hex code for the specific character.
        printf(" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e)) {
            buff[i % 16] = '.';
        } else {
            buff[i % 16] = pc[i];
        }

        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        printf("   ");
        i++;
    }

    // And print the final ASCII bit.
    printf("  %s\n", buff);
}

static void process_assh_events (struct ssh *ssh) {
  ssize_t r;

  assh_status_t err = ASSH_OK;

  do {
    //printf("event.id = %d, ", ssh->event->id);
    if (ssh->event->id < 0 || ssh->event->id > 50) {
      puts("event.id is either less than zero or greater than 50");
      sleep(5);
    }
    switch (ssh->event->id) {
      case ASSH_EVENT_SESSION_ERROR:
        fprintf(stderr, "SSH error: %s\n", assh_error_str(ssh->event->session.error.code));
        // Example: SSH error: IO error
        /* TODO This may not be the right thing to do */
        //assh_session_disconnect(ssh->session, SSH_DISCONNECT_BY_APPLICATION, NULL);
        //close(w->fd);
        //w->data->
        /* */
        assh_event_done(ssh->session, ssh->event, ASSH_OK);
        break;

      case ASSH_EVENT_KEX_HOSTKEY_LOOKUP:
        printf("Host key lookup called\n");
        struct assh_event_kex_hostkey_lookup_s *ek = &ssh->event->kex.hostkey_lookup;
        ek->accept = 1;
        assh_event_done(ssh->session, ssh->event, ASSH_OK);
        puts("Host key lookup finished");
        break;

      case ASSH_EVENT_USERAUTH_CLIENT_BANNER:
        printf("Client Banner called\n");
        struct assh_event_userauth_client_banner_s *eb = &ssh->event->userauth_client.banner;
        assert(ssh->event->id == ASSH_EVENT_USERAUTH_CLIENT_BANNER);
        assh_event_done(ssh->session, ssh->event, ASSH_OK);
        break;

      case ASSH_EVENT_USERAUTH_CLIENT_USER:
        printf("Client User called\n");
        struct assh_event_userauth_client_user_s *eu = &ssh->event->userauth_client.user;
        assh_buffer_strset(&eu->username, "sysadmin");
        assh_event_done(ssh->session, ssh->event, ASSH_OK);
        break;

      case ASSH_EVENT_USERAUTH_CLIENT_METHODS:
        printf("Client User methods called\n");
        struct assh_event_userauth_client_methods_s *ev = &ssh->event->userauth_client.methods;
        assh_buffer_strset(&ev->password, "sysadmin");
        ev->select = ASSH_USERAUTH_METHOD_PASSWORD;
        //ev->select = ASSH_USERAUTH_METHOD_PUBKEY;
        assh_event_done(ssh->session, ssh->event, ASSH_OK);
        break;

      case ASSH_EVENT_USERAUTH_CLIENT_PWCHANGE:
        printf("Client PWChange called\n");
        assh_event_done(ssh->session, ssh->event, ASSH_OK);
      case ASSH_EVENT_USERAUTH_CLIENT_KEYBOARD:
        printf("Client Keyboard called\n");
        assh_event_done(ssh->session, ssh->event, ASSH_OK);
        break;

      case ASSH_EVENT_SERVICE_START:
        printf("Service Start called\n");
        assh_bool_t conn = ssh->event->service.start.srv ==
                           &assh_service_connection;
  
        assh_event_done(ssh->session, ssh->event, ASSH_OK);
  
        if (conn) {
          /* We can send channel related requests from this point */
          assert(ssh->inter->state == ASSH_CLIENT_INTER_ST_INIT);
  
          if (asshh_inter_open_session(ssh->session, &ssh->inter->channel)) {
            puts("asshh_inter_open_session failed");
            goto err;
          }
    
          ASSH_SET_STATE(ssh->inter, state, ASSH_CLIENT_INTER_ST_SESSION);
        }
        break;
      case ASSH_EVENT_CHANNEL_OPEN:
        printf("Event Channel open request from remote host. No channel for you!\n");
        struct assh_event_request_s *er = &ssh->event->connection.request;
        er->reply = ASSH_CONNECTION_REPLY_FAILED;
        break;
      case ASSH_EVENT_CHANNEL_CONFIRMATION:
        printf("Our channel open request was confirmed by the remote host\n");
        struct assh_event_channel_confirmation_s *ev_confirm = &ssh->event->connection.channel_confirmation;

        if (ev_confirm->ch != ssh->inter->channel)
          return;

        assert(ssh->inter->state == ASSH_CLIENT_INTER_ST_SESSION);

        assh_event_done(ssh->session, ssh->event, ASSH_OK);

        if (ssh->inter->term == NULL) {
          puts("EVENT_CHANNEL_CONFIRMATION_ERR");
          goto err;
        }

        ASSH_SET_STATE(ssh->inter, state, ASSH_CLIENT_INTER_ST_PTY);

        //struct asshh_inter_pty_req_s i;
        //asshh_inter_init_pty_req(&i, ssh->inter->term, ssh->inter->char_width, ssh->inter->char_height, 0, 0, NULL);
        struct asshh_inter_subsystem_s i;
        asshh_inter_init_subsystem(&i, "netconf");

        //if (asshh_inter_send_pty_req(ssh->session, ssh->inter->channel, &ssh->inter->request, &i)) {
        if (asshh_inter_send_subsystem(ssh->session, ssh->inter->channel, &ssh->inter->request, &i)) {
          puts("EVENT_CHANNEL_CONFIRMATION_ERR");
          goto err;
        }
        break;

      case ASSH_EVENT_CHANNEL_FAILURE:
        printf("Channel Failure called\n");
          struct assh_event_channel_failure_s *ev_channel_failure = &ssh->event->connection.channel_failure;

        if (ev_channel_failure->ch != ssh->inter->channel)
          return;

        assert(ssh->inter->state == ASSH_CLIENT_INTER_ST_SESSION);

        assh_event_done(ssh->session, ssh->event, ASSH_OK);

        goto err;

      case ASSH_EVENT_CHANNEL_ABORT:
        printf("ASSH_EVENT_CHANNEL_ABORT was called.\n");
        break;

//      case ASSH_EVENT_CHANNEL_WINDOW:
//        printf("ASSH_EVENT_CHANNEL_WINDOW was called.\n");
//        break;

      case ASSH_CLIENT_INTER_ST_EXEC:
      case ASSH_EVENT_REQUEST_SUCCESS:
        printf("Request Success called\n");
        struct assh_event_request_success_s *ev_request_success = &ssh->event->connection.request_success;

        if (ev_request_success->ch != ssh->inter->channel ||
          ev_request_success->rq != ssh->inter->request)
          break;

        assh_event_done(ssh->session, ssh->event, ASSH_OK);

        switch (ssh->inter->state)
          {
          case ASSH_CLIENT_INTER_ST_PTY:
            printf("ASSH_CLIENT_INTER_ST_PTY case called\n");
//          exec:
//            if (ssh->inter->command)
//             {
//                struct asshh_inter_exec_s i;
//                assh_buffer_strset(&i.command, ssh->inter->command);
//                if (asshh_inter_send_exec(ssh->session, ssh->inter->channel, &ssh->inter->request, &i))
//                  goto err;
//                ASSH_SET_STATE(ssh->inter, state, ASSH_CLIENT_INTER_ST_EXEC);
//              }
//            else
//              {
//                if (asshh_inter_send_shell(ssh->session, ssh->inter->channel, &ssh->inter->request))
//                  goto err;
//                ASSH_SET_STATE(ssh->inter, state, ASSH_CLIENT_INTER_ST_EXEC);
//              }

            //struct asshh_inter_subsystem_s i;
            //if (asshh_inter_send_subsystem(ssh->session, ssh->inter->channel, &ssh->inter->request, &i))
            //  goto err;
            ASSH_SET_STATE(ssh->inter, state, ASSH_CLIENT_INTER_ST_EXEC);
            break;

          case ASSH_CLIENT_INTER_ST_EXEC:
            printf("ASSH_CLIENT_INTER_ST_EXEC case called\n");
            ASSH_SET_STATE(ssh->inter, state, ASSH_CLIENT_INTER_ST_OPEN);
            break;

          default:
            printf("ASSH_CLIENT_INTER_ default case called\n");
            ASSH_UNREACHABLE();
          }
        break;

      case ASSH_EVENT_REQUEST_FAILURE:
        struct assh_event_request_failure_s *ev_request_failure = &ssh->event->connection.request_failure;
        printf("Request Failure called, reason: %s\n", ev_request_failure->reason ? "REQUEST_FAILED" : "SESSION_DISCONNECTED");
        // ASSH_REQUEST_SESSION_DISCONNECTED, 0
        // ASSH_REQUEST_FAILED, 1

        if (ev_request_failure->ch != ssh->inter->channel ||
            ev_request_failure->rq != ssh->inter->request)
          break;

        assh_event_done(ssh->session, ssh->event, ASSH_OK);

        goto err;

      case ASSH_EVENT_CHANNEL_CLOSE:
        printf("Channel Close called\n");
        struct assh_event_channel_close_s *ev_channel_close = &ssh->event->connection.channel_close;

        if (ev_channel_close->ch != ssh->inter->channel)
          break;

        printf("Actually Closing channel\n");
        ASSH_SET_STATE(ssh->inter, state, ASSH_CLIENT_INTER_ST_CLOSED);
        assh_event_done(ssh->session, ssh->event, ASSH_OK);
        break;

        //asshh_client_event_inter_session(ssh->session, ssh->event, ssh->inter);
        if (ssh->inter->state == ASSH_CLIENT_INTER_ST_CLOSED)
          assh_session_disconnect(ssh->session, SSH_DISCONNECT_BY_APPLICATION, NULL);
        break;

      case ASSH_EVENT_CHANNEL_DATA:
        //printf("state = %d\n", ssh->inter->state);
        printf("Channel Data called\n");
        //printf("Read %d bytes\n", r);
        struct assh_event_channel_data_s *ec = &ssh->event->connection.channel_data;
        //struct cbor_load_result result;

        err = ASSH_OK;

        //ter = ssh->event->transport.read;
        //printf("\n%.*s\n", ec->data.size, ec->data.data);
        //free(buffer);

        ssize_t r = write(1, ec->data.data, ec->data.size);
//        switch (r) {
//          case -1:
//            r = 0;
//            if (errno == EAGAIN || errno == EWOULDBLOCK)
//              break;
//          case 0:
//            r = -1;
//            err = ASSH_ERR_IO;
//          default:
//            ec->transferred = r;
//        }

        ec->transferred = ec->data.size;
        //assh_channel_window_adjust(ec->ch, ec->transferred);
        assh_event_done(ssh->session, ssh->event, err);

        uint8_t *send_buf;
        //printf("Wrote %d bytes to stdout, ", r);
        if (strncmp(ec->data.data + ec->data.size - 14 , "</hello>]]>]]>", 14) == 0) {
          puts("\nEnd of hello seen\n");
          char *hello_string = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><hello xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\"><capabilities><capability>urn:ietf:params:netconf:base:1.0</capability></capabilities></hello>]]>]]>";
          size_t hello_length = strlen(hello_string);
          err = assh_channel_data_alloc(ssh->inter->channel, &send_buf, &hello_length, hello_length);
          memcpy(send_buf, hello_string, hello_length);
          printf("Tried allocating buffer to hello string. The result is %d\n", ASSH_STATUS(err));
          if (ASSH_STATUS(err) == ASSH_OK) {
            puts("Sending hello string");
            assh_channel_data_send(ssh->inter->channel, hello_length);
          }

          char *subscribe_string = "<rpc xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\" message-id=\"1\"><create-subscription xmlns=\"urn:ietf:params:xml:ns:netconf:notification:1.0\"><stream>exa-events</stream></create-subscription></rpc>]]>]]>";
          size_t subscribe_length = strlen(subscribe_string);
          err = assh_channel_data_alloc(ssh->inter->channel, &send_buf, &subscribe_length, subscribe_length);
          memcpy(send_buf, subscribe_string, subscribe_length);
          printf("Tried allocating buffer to subscribe string. The result is %d\n", ASSH_STATUS(err));
          if (ASSH_STATUS(err) == ASSH_OK) {
            puts("Sending subscribe string");
            assh_channel_data_send(ssh->inter->channel, subscribe_length);
          }
          //char *get_onts_string = "<rpc xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\" message-id=\"487\"><get xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\"><filter type=\"xpath\" select=\"/status/system/ont\"/></get></rpc>]]>]]>";
          //size_t get_onts_length = strlen(get_onts_string);
          //err = assh_channel_data_alloc(ssh->inter->channel, &send_buf, &get_onts_length, get_onts_length);
          //memcpy(send_buf, get_onts_string, get_onts_length);
          //printf("Tried allocating buffer to get_onts string. The result is %d\n", ASSH_STATUS(err));
          //if (ASSH_STATUS(err) == ASSH_OK) {
          //  puts("Sending onts_string string");
          //  assh_channel_data_send(ssh->inter->channel, get_onts_length);
          //}
        } else {
          puts("\nEnd of hello not seen\n");
        }

        break;

      case ASSH_EVENT_KEX_DONE:
        printf("Key exchange completed\n");
        assh_event_done(ssh->session, ssh->event, ASSH_OK);
        break;

      case ASSH_EVENT_USERAUTH_CLIENT_SUCCESS:
        printf("User authentication completed\n");
        assh_event_done(ssh->session, ssh->event, ASSH_OK);
        break;

      case ASSH_EVENT_REQUEST:
        struct assh_event_request_s *ereq = &ssh->event->connection.request;
        //printf("Received event request %.*s", ereq->type.len, ereq->type.str);
        assh_event_done(ssh->session, ssh->event, ASSH_OK);
        break;

      case ASSH_EVENT_CHANNEL_WINDOW:
        printf("An channel window event was received\n");
        assh_event_done(ssh->session, ssh->event, ASSH_OK);
        break;

      default:
        printf("Some other event happened: %d\n", ssh->event->id);
        assh_event_done(ssh->session, ssh->event, ASSH_OK);
    }

    if (assh_event_get(ssh->session, ssh->event, time(NULL)) == 0) {
      puts("result was zero. Bailing");
      ev_io_stop (loop, ssh->socket_watcher_reader);
      free(ssh->socket_watcher_reader);
      ev_io_stop (loop, ssh->socket_watcher_writer);
      free(ssh->socket_watcher_writer);
      goto out;
    }
    //printf("The next event id is %d\n", ssh->event->id);
  } while (ssh->event->id != ASSH_EVENT_READ && ssh->event->id != ASSH_EVENT_WRITE);

  //puts("Bailed out of do loop. Network interface needed");
  if (ssh->event->id == ASSH_EVENT_READ ) {
    ev_io_start(loop, ssh->socket_watcher_reader);
    ev_io_stop(loop, ssh->socket_watcher_writer);
  } else if (ssh->event->id == ASSH_EVENT_WRITE) {
    ev_io_stop(loop, ssh->socket_watcher_reader);
    ev_io_start(loop, ssh->socket_watcher_writer);
  }

  goto out;

err:
  switch (ssh->inter->state)
    {
    case ASSH_CLIENT_INTER_ST_INIT:
    case ASSH_CLIENT_INTER_ST_SESSION:
      ASSH_SET_STATE(ssh->inter, state, ASSH_CLIENT_INTER_ST_CLOSED);
      break;
    case ASSH_CLIENT_INTER_ST_PTY:
    case ASSH_CLIENT_INTER_ST_EXEC:
    case ASSH_CLIENT_INTER_ST_OPEN:
      assh_channel_close(ssh->inter->channel);
      ASSH_SET_STATE(ssh->inter, state, ASSH_CLIENT_INTER_ST_CLOSING);
      break;
    case ASSH_CLIENT_INTER_ST_CLOSING:
    case ASSH_CLIENT_INTER_ST_CLOSED:
      break;
    }
out:
  printf("*");
  return;
}

static void ssh_network_read (struct ev_loop *loop, ev_io *w, int revents) {
  struct ssh *ssh = w->data;
  int r;
  if (ssh->banner_seen == 0) {
    puts("Socket is readable, and the banner has arrived");
    char *buffer[1024];
    ssh->banner_seen = 1;
    r = read(w->fd, buffer, 1024);
    printf("%s: %.*s", ssh->call_home_remote_address, r, buffer);
    ev_io_stop(loop, ssh->socket_watcher_reader);
    ev_io_start(loop, ssh->socket_watcher_writer);
    goto out;
  }

  // We're going to assume that the library is in a READ state
  assh_status_t assh_status = ASSH_OK;
  struct assh_event_transport_read_s *transport_read;
  transport_read = &ssh->event->transport.read;
  r = read(w->fd, transport_read->buf.data, transport_read->buf.size);
  switch (r) {
    case -1:
      r = 0;
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        //break;
        assh_event_done(ssh->session, ssh->event, assh_status);
        goto out;
      }
      break;
    case 0:
      r = -1;
      assh_status = ASSH_ERR_IO;
    default:
      transport_read->transferred = r;
      break;
  }
  assh_event_done(ssh->session, ssh->event, assh_status);
  if (assh_event_get(ssh->session, ssh->event, time(NULL)) == 0) {
    // TODO: Cleanup socket
    ev_io_stop (loop, ssh->socket_watcher_reader);
    free(ssh->socket_watcher_reader);
    ev_io_stop (loop, ssh->socket_watcher_writer);
    free(ssh->socket_watcher_writer);
  }

  // Does the library need network activity?
  if (ssh->event->id == ASSH_EVENT_READ) {
    // Our read watcher is already active
    goto out;
  } else if (ssh->event->id == ASSH_EVENT_WRITE) {
    ev_io_stop (loop, ssh->socket_watcher_reader);
    ev_io_start (loop, ssh->socket_watcher_writer);
    goto out;
  }

  // There are internal assh library events to process
  process_assh_events(ssh);

out:
  return;
}

static void ssh_network_write (struct ev_loop *loop, ev_io *w, int revents) {
  struct ssh *ssh = w->data;
  int r;
  if (ssh->banner_seen == 0) {
    // This shouldn't happen
    puts("Socket is writable, but no banner has been seen");
    goto out;
  } else if (ssh->banner_seen == 1 && ssh->banner_written == 0) {
    puts("Socket is writable and banner has been seen. Send ack");
    ssh->banner_written = 1;
    //printf("\n%s: Sending: <ack>ok</ack>\n", ssh->call_home_remote_address);
    int r = write(w->fd, "<ack>ok</ack>", 13);
    goto out;
  }

  // We're going to assume that the library is in a WRITE state
  assh_status_t assh_status = ASSH_OK;
  struct assh_event_transport_write_s *transport_write = &ssh->event->transport.write;
  r = write(w->fd, transport_write->buf.data, transport_write->buf.size);
  switch (r) {
    case -1:
      r = 0;
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        //break;
        assh_event_done(ssh->session, ssh->event, assh_status);
        goto out;
      }
      break;
    case 0:
      r = -1;
      assh_status = ASSH_ERR_IO;
    default:
      transport_write->transferred = r;
      break;
  }

  assh_event_done(ssh->session, ssh->event, assh_status);
  if (assh_event_get(ssh->session, ssh->event, time(NULL)) == 0) {
    // TODO: Cleanup socket
    ev_io_stop (loop, ssh->socket_watcher_reader);
    free(ssh->socket_watcher_reader);
    ev_io_stop (loop, ssh->socket_watcher_writer);
    free(ssh->socket_watcher_writer);
  }

  // Does the library need network activity?
  if (ssh->event->id == ASSH_EVENT_WRITE) {
    // Our write watcher is already active
    goto out;
  } else if (ssh->event->id == ASSH_EVENT_READ) {
    ev_io_stop (loop, ssh->socket_watcher_writer);
    ev_io_start (loop, ssh->socket_watcher_reader);
    goto out;
  }

  // There are internal assh library events to process
  process_assh_events(ssh);

out:
  return;
}

void netconf_listener_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {
  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);
  int client_sd;
  struct ev_io *w_client = (struct ev_io*) malloc (sizeof(struct ev_io));

  if(EV_ERROR & revents) {
    perror("got invalid event");
    return;
  }

  // Accept client request
  client_sd = accept(watcher->fd, (struct sockaddr *)&client_addr, &client_len);

  if (client_sd < 0) {
    perror("accept error");
    return;
  }

  //printf("===Successfully received connection from callhome server===\n");
  //sleep(5);

  int res = getpeername(client_sd, (struct sockaddr *)&client_addr, &client_len);
  char *call_home_remote_address;
  call_home_remote_address = malloc(sizeof(char)*20);
  strcpy(call_home_remote_address, inet_ntoa(client_addr.sin_addr));

  struct assh_context_s *context;

  if (assh_context_create(&context, ASSH_CLIENT,
                          NULL, NULL, NULL, NULL) ||
      assh_service_register_default(context) ||
      assh_algo_register_default(context, ASSH_SAFETY_WEAK))
    ERROR("Unable to create an assh context.\n");

  struct assh_session_s *session;

  if (assh_session_create(context, &session))
    ERROR("Unable to create an assh session.\n");

  enum assh_userauth_methods_e *auth_methods;
  auth_methods = malloc(sizeof(enum assh_userauth_methods_e));
  *auth_methods = ASSH_USERAUTH_METHOD_PUBKEY;

  struct asshh_client_inter_session_s *inter;
  inter = malloc(sizeof(struct asshh_client_inter_session_s));
  //asshh_client_init_inter_session(inter, /*command to run*/"show version", "vt100");
  asshh_client_init_inter_session(inter, /*command to run*/"", "vt100");
  
  //struct asshh_inter_subsystem_s *inter_subsystem;
  //inter_subsystem = malloc(sizeof(struct asshh_inter_subsystem_s));
  //asshh_inter_init_subsystem(inter_subsystem, "netconf");

  struct ssh *ssh;
  ssh = malloc(sizeof(struct ssh));
  ssh->session = session;
  ssh->inter = inter;
  ssh->auth_methods = auth_methods;
  ssh->banner_seen = 0;
  ssh->banner_written = 0;
  ssh->call_home_remote_address = call_home_remote_address;
  ssh->event = malloc(sizeof(struct assh_event_s));
  if (assh_event_get(ssh->session, ssh->event, time(NULL)) == 0) {
    // Bail
    abort();
  }
  printf("Initial event state is %d\n", ssh->event->id);
  // The library starts in WRITE state, however the first network
  // event will be the banner sent by the call_home server. We
  // will handle the reader and watcher like this:
  //   Initially, only the reader_watcher will run
  //   Once the banner has been received, we'll stop the reader_watcher and start the writer_watcher
  //   On write, we will write the ack and then fall through (maybe not) to the library
  // TODO: There may be issues with partial sends and receives to resolve

  ev_io *writer_watcher;
  ev_io *reader_watcher;
  writer_watcher = malloc(sizeof(ev_io));
  reader_watcher = malloc(sizeof(ev_io));

  ev_io_init (writer_watcher, ssh_network_write, client_sd, EV_WRITE);
  ssh->socket_watcher_writer = writer_watcher;
  ev_io_init (reader_watcher, ssh_network_read, client_sd, EV_READ);
  ssh->socket_watcher_reader = reader_watcher;

  writer_watcher->data = ssh;
  reader_watcher->data = ssh;

  ev_io_start (loop, ssh->socket_watcher_reader);
  //ssh->reader_running = 1;
  //ev_io_start (loop, ssh->socket_watcher_writer);
  //ssh->writer_running = 0;
}

static void timeout_cb (EV_P_ ev_timer *w, int revents) {
  fprintf (stderr, "timeout\n");
}

ev_signal signal_watcher;
static void sigint_cb (struct ev_loop *loop, ev_signal *w, int revents) {
  puts("catch SIGINT");
  ev_break (EV_A_ EVBREAK_ALL);
  //int true = 1;
  //setsockopt(netconf_lister_fd,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int);
  //shutdown(netconf_listener_fd, SHUT_RDWR);
  const int enable = 1;
  if (setsockopt(netconf_listener_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
    perror("setsockopt(SO_REUSEADDR) failed");
  }
  close(netconf_listener_fd);
  abort();
}

int main(int argc, char **argv) {
  if (assh_deps_init())
    ERROR("initialization error\n");

  loop = EV_DEFAULT;

  struct addrinfo hints = {
    .ai_family = AF_UNSPEC,
    .ai_socktype = SOCK_STREAM,
  };

  signal(SIGPIPE, SIG_IGN);

  int status;

  struct sockaddr_in server, client;
  int len;
  int netconf_callhome_port = 4444;
  netconf_listener_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (netconf_listener_fd < 0) {
    perror("Cannot create socket");
    exit(1);
  }
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(netconf_callhome_port);
  len = sizeof(server);
  if (bind(netconf_listener_fd, (struct sockaddr *)&server, len) < 0) {
    perror("Cannot bind socket");
    exit(2);
  }
  if (listen(netconf_listener_fd, 100000) < 0) {
    perror("Listen error");
    exit(3);
  }

  struct ev_io w_accept;
  ev_io_init(&w_accept, netconf_listener_cb, netconf_listener_fd, EV_READ);
  ev_io_start(loop, &w_accept);

  //ev_timer timeout_watcher;
  //ev_timer_init (&timeout_watcher, timeout_cb, 0.5, 10.);
  //ev_timer_start (loop, &timeout_watcher);
  //printf("Set timer event\n");

  ev_signal signal_watcher;
  ev_signal_init (&signal_watcher, sigint_cb, SIGINT);
  ev_signal_init (&signal_watcher, sigint_cb, SIGHUP);
  ev_signal_start (loop, &signal_watcher);

  ev_run (loop, 0);

  abort();

  printf("Connection closed\n");

  //assh_session_release(session);
}
