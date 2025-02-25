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
#include <sys/queue.h>

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
#include <expat.h>
#include <netconf_generated.h>
#include <netconf.h>
#include <nx_gperf.h>

// TODO: Get rid of this and replace
#define ERROR(...) do { fprintf(stderr, __VA_ARGS__); exit(1); } while (0)

// TODO: Verify and document the size. Name it something better
#define BUFFER_SIZE 2048

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

//Juniper sends this (https://www.juniper.net/documentation/us/en/software/junos/netconf/topics/topic-map/netconf-call-home.html):
//MSG-ID: DEVICE-CONN-INFO\r\n
//MSG-VER: V1\r\n
//DEVICE-ID: <device-id>\r\n
//HOST-KEY: <public-host-key>\r\n
//HMAC:<HMAC(pub-SSH-host-key, <secret>)>\r\n
//
//Calix AXOS sends this:
//<version>1</version>
//<identity>
//  <mac>00:02:5d:d9:21:47</mac>
//  <serial-number>071904926728</serial-number>
//  <model-name>E7 System</model-name>
//  <source-ip>192.168.35.13</source-ip>
//</identity>
//
// And then expects:
//  <ack>ok</ack>
static void XMLCALL banner_start_element(void *data, const XML_Char *tag_name, const XML_Char **atts) {
  //printf("B<%s>B", tag_name);
}

static void XMLCALL banner_end_element(void *data, const XML_Char *tag_name) {
  struct ssh *ssh = (struct ssh *)data;
  //printf("B</%s>B", tag_name);
  //if (strncmp(tag_name, "identity", 8) == 0) {
  if (strcmp(tag_name, "identity") == 0) {
    ssh->banner_is_complete = 1;
  //} else if (strncmp(tag_name, "source-ip", 9) == 0) {
  } else if (strcmp(tag_name, "source-ip") == 0) {
    if (ssh->calix_remote_ip != NULL) {
      free(ssh->calix_remote_ip);
    }
    size_t char_buffer_len = strlen(ssh->xml_char_buffer);
    ssh->calix_remote_ip = malloc(sizeof(char) * char_buffer_len + 1);
    strncpy(ssh->calix_remote_ip, ssh->xml_char_buffer, char_buffer_len);
    ssh->calix_remote_ip[char_buffer_len] = 0;
  }

  if (ssh->xml_char_buffer != NULL) {
    free(ssh->xml_char_buffer);
    ssh->xml_char_buffer = NULL;
  }
}

static void XMLCALL banner_char_handler(void *data, const XML_Char *s, int len) {
  struct ssh *ssh = (struct ssh *)data;
  if (ssh->xml_char_buffer == NULL) {
    ssh->xml_char_buffer = malloc(sizeof(char)*len + 1);
    strncpy(ssh->xml_char_buffer, s, len);
    ssh->xml_char_buffer_len = len;
    // zero terminate
    ssh->xml_char_buffer[ssh->xml_char_buffer_len] = 0;
  } else {
    ssh->xml_char_buffer = realloc(ssh->xml_char_buffer, ssh->xml_char_buffer_len + len + 1);
    strncpy(ssh->xml_char_buffer+ssh->xml_char_buffer_len, s, len);
    ssh->xml_char_buffer_len += len;
    // zero terminate
    ssh->xml_char_buffer[ssh->xml_char_buffer_len] = 0;
  }
}

static void XMLCALL hello_start_element(void *data, const XML_Char *tag_name, const XML_Char **atts) {
}

static void XMLCALL hello_end_element(void *data, const XML_Char *tag_name) {
  struct ssh *ssh = (struct ssh *)data;
  if (strncmp(tag_name, "hello", 5) == 0) {
    struct ssh *ssh = (struct ssh *)data;
    ssh->hello_end_tag_seen = 1;
    //printf("H</%s>H", tag_name);
  }
}

static void XMLCALL hello_char_handler(void *data, const XML_Char *s, int len) {
}

static void XMLCALL message_start_element(void *data, const XML_Char *tag_name, const XML_Char **atts) {
  struct ssh *ssh = (struct ssh *)data;
  size_t tag_len = strlen(tag_name);

  struct tag_stack_tailq *tag_tailq = malloc(sizeof(*tag_tailq));
  TAILQ_INSERT_HEAD(ssh->tag_stack_head, tag_tailq, tags);

  const struct nx_parse *tag_parsed = in_word_set_nx(tag_name, tag_len);
  if (tag_parsed != NULL) {
    tag_tailq->tag_type = tag_parsed->tag_type;
  } else {
    tag_tailq->tag_type = XML_TAG_TYPE_UNKNOWN;
    size_t size_of_text_to_copy = (tag_len > 50) ? 50 : tag_len;
    strncpy(tag_tailq->unknown_tag_text, tag_name, size_of_text_to_copy);
    // Null terminate string
    tag_tailq->unknown_tag_text[size_of_text_to_copy] = 0;
  }

  if (atts[0] && strcmp(atts[0], "xmlns") == 0) {
    size_t xmlns_len = strlen(atts[1]);
    const struct nx_xmlns_parse *namespace_parsed = in_word_set_nx_xmlns(atts[1], xmlns_len);
    if (namespace_parsed != NULL) {
      tag_tailq->namespace = namespace_parsed->namespace;
    } else {
      tag_tailq->namespace = XML_NAMESPACE_UNKNOWN;
    }
  }

  if (tag_tailq->tag_type == XML_TAG_TYPE_ONT 
      && tag_tailq->namespace == XML_NAMESPACE_CALIX_EXA_GPON_INTERFACE_BASE) {
    //puts("Storage ONT element encountered");
    ssh->calix_storage_object = CALIX_STORE_ONT_DETAIL;
    ssh->cbor_root = cbor_new_indefinite_map();
    if (ssh->cbor_root == NULL) {
      // TODO: Do proper error checking
      abort();
    } 
    ssh->cbor_current_item = ssh->cbor_root;
    // TODO: Do some error checking
    bool success = cbor_map_add(ssh->cbor_current_item, (struct cbor_pair){
      .key = cbor_move(cbor_build_string("calix_system_ip")),
      .value = cbor_move(cbor_build_string(ssh->calix_remote_ip))});
    success = cbor_map_add(ssh->cbor_current_item, (struct cbor_pair){
      .key = cbor_move(cbor_build_string("message_type")),
      .value = cbor_move(cbor_build_string("ont_detail"))});
  } else if (tag_tailq->tag_type == XML_TAG_TYPE_USER_LOGIN
      && tag_tailq->namespace == XML_NAMESPACE_CALIX_EXA_BASE) {
    ssh->calix_storage_object = CALIX_STORE_USER_LOGIN;
    ssh->cbor_root = cbor_new_indefinite_map();
    if (ssh->cbor_root == NULL) {
      // TODO: Do proper error checking
      abort();
    } 
    ssh->cbor_current_item = ssh->cbor_root;
    // TODO: Do some error checking
    bool success = cbor_map_add(ssh->cbor_current_item, (struct cbor_pair){
      .key = cbor_move(cbor_build_string("calix_system_ip")),
      .value = cbor_move(cbor_build_string(ssh->calix_remote_ip))});
    success = cbor_map_add(ssh->cbor_current_item, (struct cbor_pair){
      .key = cbor_move(cbor_build_string("message_type")),
      .value = cbor_move(cbor_build_string("user_login"))});
  } else if (tag_tailq->tag_type == XML_TAG_TYPE_USER_LOGOUT
      && tag_tailq->namespace == XML_NAMESPACE_CALIX_EXA_BASE) {
    ssh->calix_storage_object = CALIX_STORE_USER_LOGOUT;
    ssh->cbor_root = cbor_new_indefinite_map();
    if (ssh->cbor_root == NULL) {
      // TODO: Do proper error checking
      abort();
    } 
    ssh->cbor_current_item = ssh->cbor_root;
    // TODO: Do some error checking
    bool success = cbor_map_add(ssh->cbor_current_item, (struct cbor_pair){
      .key = cbor_move(cbor_build_string("calix_system_ip")),
      .value = cbor_move(cbor_build_string(ssh->calix_remote_ip))});
    success = cbor_map_add(ssh->cbor_current_item, (struct cbor_pair){
      .key = cbor_move(cbor_build_string("message_type")),
      .value = cbor_move(cbor_build_string("user_logout"))});
  } else if (tag_tailq->tag_type == XML_TAG_ONT_US_SDBER
      && tag_tailq->namespace == XML_NAMESPACE_CALIX_EXA_GPON_INTERFACE_BASE) {
    ssh->calix_storage_object = CALIX_STORE_ONT_US_SDBER;
    ssh->storage_struct = malloc(sizeof(struct ont_us_sdber_table));
    ssh->cbor_root = cbor_new_indefinite_map();
    if (ssh->cbor_root == NULL) {
      // TODO: Do proper error checking
      abort();
    } 
    ssh->cbor_current_item = ssh->cbor_root;
    // TODO: Do some error checking
    //bool success = cbor_map_add(ssh->cbor_current_item, (struct cbor_pair){
    //  .key = cbor_move(cbor_build_string("calix_system_ip")),
    //  .value = cbor_move(cbor_build_string(ssh->calix_remote_ip))});
    //success = cbor_map_add(ssh->cbor_current_item, (struct cbor_pair){
    //  .key = cbor_move(cbor_build_string("message_type")),
    //  .value = cbor_move(cbor_build_string("user_logout"))});
  }
}

static void XMLCALL message_end_element(void *data, const XML_Char *tag_name) {
  struct ssh *ssh = (struct ssh *)data;
  struct tag_stack_tailq *tag_tailq;
  tag_tailq = TAILQ_FIRST(ssh->tag_stack_head);

  bool success = true;
  if (ssh->calix_storage_object != CALIX_STORE_NONE) {
    switch (ssh->calix_storage_object) {
      case (CALIX_STORE_ONT_DETAIL):
        switch (tag_tailq->tag_type) {
          case (XML_TAG_TYPE_MFG_SERIAL_NUMBER):
            success &= cbor_map_add(ssh->cbor_current_item, (struct cbor_pair){
              .key = cbor_move(cbor_build_string("serial_number")),
              .value = cbor_move(cbor_build_string(ssh->xml_char_buffer))});
            break;
          case (XML_TAG_TYPE_ONT_ID):
            success &= cbor_map_add(ssh->cbor_current_item, (struct cbor_pair){
              .key = cbor_move(cbor_build_string("ont_id")),
              .value = cbor_move(cbor_build_string(ssh->xml_char_buffer))});
            break;
        }
        break;
      case (CALIX_STORE_USER_LOGIN):
        switch (tag_tailq->tag_type) {
          case (XML_TAG_TYPE_SESSION_IP):
            success &= cbor_map_add(ssh->cbor_current_item, (struct cbor_pair){
              .key = cbor_move(cbor_build_string("remote_ip")),
              .value = cbor_move(cbor_build_string(ssh->xml_char_buffer))});
            break;
          case (XML_TAG_TYPE_USER_NAME):
            success &= cbor_map_add(ssh->cbor_current_item, (struct cbor_pair){
              .key = cbor_move(cbor_build_string("user_name")),
              .value = cbor_move(cbor_build_string(ssh->xml_char_buffer))});
            break;
          case (XML_TAG_TYPE_SESSION_MANAGER):
            success &= cbor_map_add(ssh->cbor_current_item, (struct cbor_pair){
              .key = cbor_move(cbor_build_string("connection_type")),
              .value = cbor_move(cbor_build_string(ssh->xml_char_buffer))});
            break;
        }
        break;
      case (CALIX_STORE_USER_LOGOUT):
        switch (tag_tailq->tag_type) {
          case (XML_TAG_TYPE_SESSION_IP):
            success &= cbor_map_add(ssh->cbor_current_item, (struct cbor_pair){
              .key = cbor_move(cbor_build_string("remote_ip")),
              .value = cbor_move(cbor_build_string(ssh->xml_char_buffer))});
            break;
          case (XML_TAG_TYPE_USER_NAME):
            success &= cbor_map_add(ssh->cbor_current_item, (struct cbor_pair){
              .key = cbor_move(cbor_build_string("user_name")),
              .value = cbor_move(cbor_build_string(ssh->xml_char_buffer))});
            break;
          case (XML_TAG_TYPE_SESSION_MANAGER):
            success &= cbor_map_add(ssh->cbor_current_item, (struct cbor_pair){
              .key = cbor_move(cbor_build_string("connection_type")),
              .value = cbor_move(cbor_build_string(ssh->xml_char_buffer))});
            break;
        }
        break;
      case (CALIX_STORE_ONT_US_SDBER):
        struct ont_us_sdber_table *sdber_table = ssh->storage_struct;
        switch (tag_tailq->tag_type) {
          case (XML_TAG_TYPE_ONT_ID):
            sdber_table->ont_id = atoi(ssh->xml_char_buffer);
            break;
          case (XML_TAG_TYPE_SHELF):
            sdber_table->shelf = atoi(ssh->xml_char_buffer);
            break;
          case (XML_TAG_TYPE_SLOT):
            sdber_table->slot = atoi(ssh->xml_char_buffer);
            break;
          case (XML_TAG_TYPE_PORT):
            sdber_table->port = malloc(sizeof(char) * ssh->char_buffer_len + 1);
            strcpy(sdber_table->port, ssh->xml_char_buffer, ssh->char_buffer_len);
            sdber_table-port[ssh->char_buffer_len] = 0;
            break;
        }
        break;
    }
  }

  if (ssh->xml_char_buffer != NULL) {
    free(ssh->xml_char_buffer);
    ssh->xml_char_buffer = NULL;
  }

  if (ssh->calix_storage_object == CALIX_STORE_ONT_DETAIL
      && tag_tailq->tag_type == XML_TAG_TYPE_ONT 
      && tag_tailq->namespace == XML_NAMESPACE_CALIX_EXA_GPON_INTERFACE_BASE) {
    //puts("Storage ONT element ended");
    ssh->calix_storage_object = CALIX_STORE_NONE;
    //ssh->cbor_root = cbor_new_indefinite_map();
    unsigned char* cbor_buffer;
    size_t cbor_buffer_size;
    cbor_serialize_alloc(ssh->cbor_root, &cbor_buffer, &cbor_buffer_size);
    cJSON* cjson_item = cbor_to_cjson(ssh->cbor_root);
    char* json_string = cJSON_PrintUnformatted(cjson_item);
    printf("%s\n", json_string);
    free(json_string);
    cJSON_Delete(cjson_item);
    free(cbor_buffer);
    cbor_decref(&ssh->cbor_root);
  } else if (ssh->calix_storage_object == CALIX_STORE_USER_LOGIN
      && tag_tailq->tag_type == XML_TAG_TYPE_USER_LOGIN
      && tag_tailq->namespace == XML_NAMESPACE_CALIX_EXA_BASE) {
    //puts("Storage ONT element ended");
    ssh->calix_storage_object = CALIX_STORE_NONE;
    //ssh->cbor_root = cbor_new_indefinite_map();
    unsigned char* cbor_buffer;
    size_t cbor_buffer_size;
    cbor_serialize_alloc(ssh->cbor_root, &cbor_buffer, &cbor_buffer_size);
    cJSON* cjson_item = cbor_to_cjson(ssh->cbor_root);
    char* json_string = cJSON_PrintUnformatted(cjson_item);
    printf("%s\n", json_string);
    free(json_string);
    cJSON_Delete(cjson_item);
    free(cbor_buffer);
    cbor_decref(&ssh->cbor_root);
  } else if (ssh->calix_storage_object == CALIX_STORE_USER_LOGOUT
      && tag_tailq->tag_type == XML_TAG_TYPE_USER_LOGOUT
      && tag_tailq->namespace == XML_NAMESPACE_CALIX_EXA_BASE) {
    //puts("Storage ONT element ended");
    ssh->calix_storage_object = CALIX_STORE_NONE;
    //ssh->cbor_root = cbor_new_indefinite_map();
    unsigned char* cbor_buffer;
    size_t cbor_buffer_size;
    cbor_serialize_alloc(ssh->cbor_root, &cbor_buffer, &cbor_buffer_size);
    cJSON* cjson_item = cbor_to_cjson(ssh->cbor_root);
    char* json_string = cJSON_PrintUnformatted(cjson_item);
    printf("%s\n", json_string);
    free(json_string);
    cJSON_Delete(cjson_item);
    free(cbor_buffer);
    cbor_decref(&ssh->cbor_root);
  }

  if (ssh->storage_struct != NULL) {
    free(ssh->storage_struct);
    ssh->storage_struct = NULL;
  }

  TAILQ_REMOVE(ssh->tag_stack_head, tag_tailq, tags);
  free(tag_tailq);
}

static void XMLCALL message_char_handler(void *data, const XML_Char *s, int len) {
  struct ssh *ssh = (struct ssh *)data;

  if (ssh->calix_storage_object != CALIX_STORE_NONE) {
    if (ssh->xml_char_buffer == NULL) {
      ssh->xml_char_buffer = malloc(sizeof(char)*len + 1);
      strncpy(ssh->xml_char_buffer, s, len);
      ssh->xml_char_buffer_len = len;
      // zero terminate
      ssh->xml_char_buffer[ssh->xml_char_buffer_len] = 0;
    } else {
      ssh->xml_char_buffer = realloc(ssh->xml_char_buffer, ssh->xml_char_buffer_len + len + 1);
      strncpy(ssh->xml_char_buffer+ssh->xml_char_buffer_len, s, len);
      ssh->xml_char_buffer_len += len;
      // zero terminate
      ssh->xml_char_buffer[ssh->xml_char_buffer_len] = 0;
    }
  }
}

static void process_assh_events (struct ssh *ssh) {
  ssize_t r;

  assh_status_t err = ASSH_OK;

  do {
    //printf("event.id = %d, ", ssh->event->id);
    if (ssh->event->id < 0 || ssh->event->id > 50) {
      puts("event.id is either less than zero or greater than 50");
      //sleep(5);
      abort();
    }
    switch (ssh->event->id) {
      case ASSH_EVENT_SESSION_ERROR:
        fprintf(stderr, "SSH error: %s\n", assh_error_str(ssh->event->session.error.code));
        // Example: SSH error: IO error
        /* TODO This may not be the right thing to do */
        //assh_session_disconnect(ssh->session, SSH_DISCONNECT_BY_APPLICATION, NULL);
        //close(w->fd);
        /* */
        assh_event_done(ssh->session, ssh->event, ASSH_OK);
        break;

      case ASSH_EVENT_KEX_HOSTKEY_LOOKUP:
        //printf("Host key lookup called\n");
        struct assh_event_kex_hostkey_lookup_s *ek = &ssh->event->kex.hostkey_lookup;
        // TODO: Actually do some verification here. This just accepts all keys
        ek->accept = 1;
        assh_event_done(ssh->session, ssh->event, ASSH_OK);
        break;

      case ASSH_EVENT_USERAUTH_CLIENT_BANNER:
        //printf("Client Banner called\n");
        struct assh_event_userauth_client_banner_s *eb = &ssh->event->userauth_client.banner;
        assert(ssh->event->id == ASSH_EVENT_USERAUTH_CLIENT_BANNER);
        assh_event_done(ssh->session, ssh->event, ASSH_OK);
        break;

      case ASSH_EVENT_USERAUTH_CLIENT_USER:
        //printf("Client User called\n");
        struct assh_event_userauth_client_user_s *eu = &ssh->event->userauth_client.user;
        assh_buffer_strset(&eu->username, "sysadmin");
        assh_event_done(ssh->session, ssh->event, ASSH_OK);
        break;

      case ASSH_EVENT_USERAUTH_CLIENT_METHODS:
        //printf("Client User methods called\n");
        struct assh_event_userauth_client_methods_s *ev = &ssh->event->userauth_client.methods;
        assh_buffer_strset(&ev->password, "sysadmin");
        ev->select = ASSH_USERAUTH_METHOD_PASSWORD;
        //ev->select = ASSH_USERAUTH_METHOD_PUBKEY;
        assh_event_done(ssh->session, ssh->event, ASSH_OK);
        break;

      case ASSH_EVENT_USERAUTH_CLIENT_PWCHANGE:
        //printf("Client PWChange called\n");
        // TODO: Is this valid in a Netconf session?
        assh_event_done(ssh->session, ssh->event, ASSH_OK);
        break;

      case ASSH_EVENT_USERAUTH_CLIENT_KEYBOARD:
        //printf("Client Keyboard called\n");
        // TODO: Handle this
        assh_event_done(ssh->session, ssh->event, ASSH_OK);
        break;

      case ASSH_EVENT_SERVICE_START:
        //printf("Service Start called\n");
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
        //printf("Our channel open request was confirmed by the remote host\n");
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

        // TODO: This runs on each new connection. Figure out how to run it once
        // or move it to the accept callback near the ssh-> init.
        struct asshh_inter_subsystem_s i;
        asshh_inter_init_subsystem(&i, "netconf");

        if (asshh_inter_send_subsystem(ssh->session, ssh->inter->channel, &ssh->inter->request, &i)) {
          puts("EVENT_CHANNEL_CONFIRMATION_ERR");
          goto err;
        }
        break;

      case ASSH_EVENT_CHANNEL_FAILURE:
        //printf("Channel Failure called\n");
          struct assh_event_channel_failure_s *ev_channel_failure = &ssh->event->connection.channel_failure;

        if (ev_channel_failure->ch != ssh->inter->channel)
          return;

        assert(ssh->inter->state == ASSH_CLIENT_INTER_ST_SESSION);

        assh_event_done(ssh->session, ssh->event, ASSH_OK);

        goto err;

      case ASSH_EVENT_CHANNEL_ABORT:
        //printf("ASSH_EVENT_CHANNEL_ABORT was called.\n");
        assh_event_done(ssh->session, ssh->event, ASSH_OK);
        break;

//      case ASSH_EVENT_CHANNEL_WINDOW:
//        printf("ASSH_EVENT_CHANNEL_WINDOW was called.\n");
//        break;

      case ASSH_CLIENT_INTER_ST_EXEC:
        // TODO: Do something here or remove.
        assh_event_done(ssh->session, ssh->event, ASSH_OK);
        break;

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
            //printf("ASSH_CLIENT_INTER_ST_PTY case called\n");
            ASSH_SET_STATE(ssh->inter, state, ASSH_CLIENT_INTER_ST_EXEC);
            break;

          case ASSH_CLIENT_INTER_ST_EXEC:
            //printf("ASSH_CLIENT_INTER_ST_EXEC case called\n");
            ASSH_SET_STATE(ssh->inter, state, ASSH_CLIENT_INTER_ST_OPEN);
            break;

          default:
            //printf("ASSH_CLIENT_INTER_ default case called\n");
            ASSH_UNREACHABLE();
          }
        break;

      case ASSH_EVENT_REQUEST_FAILURE:
        struct assh_event_request_failure_s *ev_request_failure = &ssh->event->connection.request_failure;
        //printf("Request Failure called, reason: %s\n", ev_request_failure->reason ? "REQUEST_FAILED" : "SESSION_DISCONNECTED");
        // ASSH_REQUEST_SESSION_DISCONNECTED, 0
        // ASSH_REQUEST_FAILED, 1

        if (ev_request_failure->ch != ssh->inter->channel ||
            ev_request_failure->rq != ssh->inter->request)
          break;

        assh_event_done(ssh->session, ssh->event, ASSH_OK);

        goto err;

      case ASSH_EVENT_CHANNEL_CLOSE:
        //printf("Channel Close called\n");
        struct assh_event_channel_close_s *ev_channel_close = &ssh->event->connection.channel_close;

        if (ev_channel_close->ch != ssh->inter->channel)
          break;

        ASSH_SET_STATE(ssh->inter, state, ASSH_CLIENT_INTER_ST_CLOSED);
        assh_event_done(ssh->session, ssh->event, ASSH_OK);
        break;

        if (ssh->inter->state == ASSH_CLIENT_INTER_ST_CLOSED)
          assh_session_disconnect(ssh->session, SSH_DISCONNECT_BY_APPLICATION, NULL);
        break;

      case ASSH_EVENT_CHANNEL_DATA:
        //printf("Channel Data called\n");
        struct assh_event_channel_data_s *ec = &ssh->event->connection.channel_data;

        err = ASSH_OK;

        // Disregard any I/O errors writing to debug file for now
        //ssize_t debug_length = write(ssh->debug_file, ec->data.data, ec->data.size);
        ssize_t debug_write_length = fwrite(ec->data.data, ec->data.size, 1, ssh->debug_file);

        if (ssh->hello_is_complete == 0) {
          XML_Parse(ssh->hello_parser, ec->data.data, ec->data.size, 0);
          // TODO: Account for a buffer split within the ]]>]]>
          if (XML_GetErrorCode(ssh->hello_parser) == XML_ERROR_JUNK_AFTER_DOC_ELEMENT &&
              ec->data.size >= 6 &&
              strncmp(ec->data.data + ec->data.size-6, "]]>]]>", 6) == 0 &&
              ssh->hello_end_tag_seen == 1) {
            XML_ParserFree(ssh->hello_parser);
            ssh->hello_is_complete = 1;
            // TODO: Handle the case where the first operational message is stacked onto the
            // end of the hello message in the same buffer (by passing it to the message parser)
          } else if (XML_GetErrorCode(ssh->hello_parser) != XML_ERROR_NONE) {
            // TODO: Bail and close connection
            puts("There was an error in the hello");
            printf("\nParse error at %llu, %llu: %s\n",
                   XML_GetCurrentLineNumber(ssh->hello_parser),
                   XML_GetCurrentByteIndex(ssh->hello_parser),
                   XML_ErrorString(XML_GetErrorCode(ssh->hello_parser)));
            printf("ec->data.size = %llu\n", ec->data.size);
            printf("ssh->hello_is_complete = %d\n", ssh->hello_is_complete);
            sleep(60);
          } else {
            // Incomplete hello buffer. Fall through and continue parsing with the next packet
          }
        } else if (ssh->hello_is_complete == 1 && ssh->hello_et_al_sent == 1) {
          // TODO: Operational message. Figure out how to handle multiple messages in the same buffer
          XML_Parse(ssh->message_parser, ec->data.data, ec->data.size, 0);
          // TODO: Account for a buffer split in the middle of the ]]>]]>
          if (XML_GetErrorCode(ssh->message_parser) == XML_ERROR_JUNK_AFTER_DOC_ELEMENT &&
              ec->data.size >= 6 &&
              strncmp(ec->data.data + ec->data.size-6, "]]>]]>", 6) == 0) {
            XML_ParserReset(ssh->message_parser, NULL);
            XML_SetElementHandler(ssh->message_parser, message_start_element, message_end_element);
            XML_SetCharacterDataHandler(ssh->message_parser, message_char_handler);
            XML_SetUserData(ssh->message_parser, (void *)ssh);
            // TODO: Handle the case where the first operational message is stacked
            // onto the end of the hello message (by passing it to the message parser)
          } else if (XML_GetErrorCode(ssh->message_parser) != XML_ERROR_NONE) {
            // TODO: Bail and close connection
            puts("There was an error in an operational message");
            //printf("data.size=%d. Remaining data in buffer is %.*s\n", ec->data.size, ec->data.size - XML_GetCurrentByteIndex(ssh->message_parser), ec->data.data+XML_GetCurrentByteIndex(ssh->message_parser));
            printf("data.size=%d. The last 6 bytes of the buffer are %.*s\n", ec->data.size, 6, ec->data.data + ec->data.size-6);
            printf("\nParse error at %llu, %llu: %s\n",
                   XML_GetCurrentLineNumber(ssh->message_parser),
                   XML_GetCurrentByteIndex(ssh->message_parser),
                   XML_ErrorString(XML_GetErrorCode(ssh->message_parser)));
            XML_ParserReset(ssh->message_parser, NULL);
            XML_SetElementHandler(ssh->message_parser, message_start_element, message_end_element);
            XML_SetCharacterDataHandler(ssh->message_parser, message_char_handler);
            XML_SetUserData(ssh->message_parser, (void *)ssh);
            sleep(5);
          } else {
            // Incomplete operational message. Fall through and continue parsing with the next packet
          }
        }

        //assh_channel_window_adjust(ec->ch, ec->transferred);
        ec->transferred = ec->data.size;
        assh_event_done(ssh->session, ssh->event, err);

        if (ssh->hello_is_complete == 1 && ssh->hello_et_al_sent == 0) {
          // Send back hello, et al
          uint8_t *send_buf;
          char *hello_string = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><hello xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\"><capabilities><capability>urn:ietf:params:netconf:base:1.0</capability></capabilities></hello>]]>]]>";
          size_t hello_length = strlen(hello_string);
          err = assh_channel_data_alloc(ssh->inter->channel, &send_buf, &hello_length, hello_length);
          memcpy(send_buf, hello_string, hello_length);
          printf("Tried allocating buffer to hello string. The result is %d\n", ASSH_STATUS(err));
          if (ASSH_STATUS(err) == ASSH_OK) {
            puts("Sending hello string");
            assh_channel_data_send(ssh->inter->channel, hello_length);
          }

          // Subscribe to notifications
          char *subscribe_string = "<rpc xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\" message-id=\"1\"><create-subscription xmlns=\"urn:ietf:params:xml:ns:netconf:notification:1.0\"><stream>exa-events</stream></create-subscription></rpc>]]>]]>";
          size_t subscribe_length = strlen(subscribe_string);
          err = assh_channel_data_alloc(ssh->inter->channel, &send_buf, &subscribe_length, subscribe_length);
          memcpy(send_buf, subscribe_string, subscribe_length);
          printf("Tried allocating buffer to subscribe string. The result is %d\n", ASSH_STATUS(err));
          if (ASSH_STATUS(err) == ASSH_OK) {
            puts("Sending subscribe string");
            assh_channel_data_send(ssh->inter->channel, subscribe_length);
          }

          // Get the state of all ONTs
          char *get_onts_string = "<rpc xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\" message-id=\"487\"><get xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\"><filter type=\"xpath\" select=\"/status/system/ont\"/></get></rpc>]]>]]>";
          size_t get_onts_length = strlen(get_onts_string);
          err = assh_channel_data_alloc(ssh->inter->channel, &send_buf, &get_onts_length, get_onts_length);
          memcpy(send_buf, get_onts_string, get_onts_length);
          printf("Tried allocating buffer to get_onts string. The result is %d\n", ASSH_STATUS(err));
          if (ASSH_STATUS(err) == ASSH_OK) {
            puts("Sending onts_string string");
            assh_channel_data_send(ssh->inter->channel, get_onts_length);
          }
          ssh->hello_et_al_sent = 1;
        }
        break;

      case ASSH_EVENT_KEX_DONE:
        //printf("Key exchange completed\n");
        assh_event_done(ssh->session, ssh->event, ASSH_OK);
        break;

      case ASSH_EVENT_USERAUTH_CLIENT_SUCCESS:
        //printf("User authentication completed\n");
        assh_event_done(ssh->session, ssh->event, ASSH_OK);
        break;

      case ASSH_EVENT_REQUEST:
        struct assh_event_request_s *ereq = &ssh->event->connection.request;
        //printf("Received event request %.*s", ereq->type.len, ereq->type.str);
        assh_event_done(ssh->session, ssh->event, ASSH_OK);
        break;

      case ASSH_EVENT_CHANNEL_WINDOW:
        //printf("An channel window event was received\n");
        assh_event_done(ssh->session, ssh->event, ASSH_OK);
        break;

      default:
        //printf("Some other event happened: %d\n", ssh->event->id);
        assh_event_done(ssh->session, ssh->event, ASSH_OK);
    }

    if (assh_event_get(ssh->session, ssh->event, time(NULL)) == 0) {
      puts("result was zero. Bailing");
      ev_io_stop (loop, ssh->socket_watcher_reader);
      //free(ssh->socket_watcher_reader);
      ev_io_stop (loop, ssh->socket_watcher_writer);
      //free(ssh->socket_watcher_writer);
      goto out;
    }
    //printf("The next event id is %d\n", ssh->event->id);
  } while (ssh->event->id != ASSH_EVENT_READ && ssh->event->id != ASSH_EVENT_WRITE);

  //puts("Fell out of do loop. Network interface needed");
  if (ssh->event->id == ASSH_EVENT_READ ) {
    ev_io_stop(loop, ssh->socket_watcher_writer);
    ev_io_start(loop, ssh->socket_watcher_reader);
  } else if (ssh->event->id == ASSH_EVENT_WRITE) {
    ev_io_stop(loop, ssh->socket_watcher_reader);
    ev_io_start(loop, ssh->socket_watcher_writer);
  }

  goto out;

// TODO: This is interactive specific and should probably go
// somewhere else
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
  return;
}

static void ssh_network_read (struct ev_loop *loop, ev_io *w, int revents) {
  struct ssh *ssh = w->data;
  int r;

  if (ssh->banner_is_complete == 0) {
    //puts("Socket is readable, and the banner has arrived");
    //void *buffer = XML_GetBuffer(ssh->banner_parser, 2048);
    //if (buffer == NULL) {
    //  // TODO: Handle failure
    //}
    char buffer[2048];
    r = read(w->fd, buffer, 2048);
    uint64_t buffer_index = 0;
    //printf("%s: %.*s", ssh->call_home_remote_address, r, buffer);
    // TODO: Close connection on junk/error
    XML_Parse(ssh->banner_parser, buffer, r, 0);
    //XML_GetErrorCode(ssh->banner_parser) == XML_ERROR_JUNK_AFTER_DOC_ELEMENT
    //XML_GetErrorCode(ssh->banner_parser) != XML_ERROR_NONE
    //XML_ErrorString(XML_GetErrorCode(ssh->banner_parser)
    //XML_GetCurrentByteIndex(ssh->banner_parser)
    while (XML_GetErrorCode(ssh->banner_parser) == XML_ERROR_JUNK_AFTER_DOC_ELEMENT) {
      //printf("\nParse error at %llu, %llu: %s\n",
      //       XML_GetCurrentLineNumber(ssh->banner_parser),
      //       XML_GetCurrentByteIndex(ssh->banner_parser),
      //       XML_ErrorString(XML_GetErrorCode(ssh->banner_parser)));
      buffer_index += XML_GetCurrentByteIndex(ssh->banner_parser);
      printf("buffer_index = %d\n", buffer_index);
      //XML_ParserFree(ssh->banner_parser);
      //ssh->banner_parser = XML_ParserCreate(NULL);
      XML_ParserReset(ssh->banner_parser, NULL);
      XML_SetElementHandler(ssh->banner_parser, banner_start_element, banner_end_element);
      XML_SetCharacterDataHandler(ssh->banner_parser, banner_char_handler);
      XML_SetUserData(ssh->banner_parser, (void *)ssh);
      XML_Parse(ssh->banner_parser, buffer+buffer_index, r-buffer_index, 0);
    }

    if (ssh->banner_is_complete == 1) {
      if (XML_GetErrorCode(ssh->banner_parser) == XML_ERROR_NONE) {
        XML_Parse(ssh->banner_parser, NULL, 0, 1);
      }
      if (XML_GetErrorCode(ssh->banner_parser) != XML_ERROR_NONE) {
        // TODO: Bail and close connection
      }
      XML_ParserFree(ssh->banner_parser);
      // We need to send the ack, which is done in ssh_network_write
      ev_io_stop(loop, ssh->socket_watcher_reader);
      ev_io_start(loop, ssh->socket_watcher_writer);
    } else {
      if (XML_GetErrorCode(ssh->banner_parser) != XML_ERROR_NONE) {
        // TODO: Unknown condition. Bail
        goto out;
      }
    }
    goto out;
  }

  // We're going to assume that the library is in a READ state
  // TODO: Add a santity check and bail if things don't make sense
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
    // TODO: Cleanup socket. The frees may cause segfault. Test this
    ev_io_stop (loop, ssh->socket_watcher_reader);
    //free(ssh->socket_watcher_reader);
    ev_io_stop (loop, ssh->socket_watcher_writer);
    //free(ssh->socket_watcher_writer);
  }

  // Does the library need network activity?
  if (ssh->event->id == ASSH_EVENT_READ) {
    // Our read watcher is already active
  } else if (ssh->event->id == ASSH_EVENT_WRITE) {
    ev_io_stop (loop, ssh->socket_watcher_reader);
    ev_io_start (loop, ssh->socket_watcher_writer);
  } else {
    // There are internal assh library events to process
    process_assh_events(ssh);
  }

out:
  return;
}

static void ssh_network_write (struct ev_loop *loop, ev_io *w, int revents) {
  struct ssh *ssh = w->data;
  int r;
  if (ssh->banner_is_complete == 0) {
    // This shouldn't happen
    puts("Socket is writable, but no banner has been seen");
    goto out;
  } else if (ssh->banner_is_complete == 1 && ssh->banner_ack_is_complete == 0) {
    //puts("Socket is writable and banner has been seen. Send ack");
    //printf("\n%s: Sending: <ack>ok</ack>\n", ssh->call_home_remote_address);
    // TODO: Add error checking and support partial writes
    r = write(w->fd, "<ack>ok</ack>", 13);
    ssh->banner_ack_is_complete = 1;
    goto out;
  }

  // We're going to assume that the library is in a WRITE state
  // TODO: Add a santity check and bail if things don't make sense
  assh_status_t assh_status = ASSH_OK;
  struct assh_event_transport_write_s *transport_write = &ssh->event->transport.write;
  r = write(w->fd, transport_write->buf.data, transport_write->buf.size);
  switch (r) {
    case -1:
      r = 0;
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        //break;
        // TODO: I'm not sure this is right. Verify
        // We should at least report 0 bytes written if we report a done, or do neither
        transport_write->transferred = r;
        assh_event_done(ssh->session, ssh->event, assh_status);
        goto out;
      }
      break;
    case 0:
      r = -1;
      // TODO: Is the library expecting transferred to be populated?
      assh_status = ASSH_ERR_IO;
    default:
      transport_write->transferred = r;
      break;
  }

  assh_event_done(ssh->session, ssh->event, assh_status);
  if (assh_event_get(ssh->session, ssh->event, time(NULL)) == 0) {
    // TODO: Cleanup socket. The frees may cause segfault. Test this
    ev_io_stop (loop, ssh->socket_watcher_reader);
    //free(ssh->socket_watcher_reader);
    ev_io_stop (loop, ssh->socket_watcher_writer);
    //free(ssh->socket_watcher_writer);
  }

  // Does the library need network activity?
  if (ssh->event->id == ASSH_EVENT_WRITE) {
    // Our write watcher is already active
  } else if (ssh->event->id == ASSH_EVENT_READ) {
    ev_io_stop (loop, ssh->socket_watcher_writer);
    ev_io_start (loop, ssh->socket_watcher_reader);
  } else {
    // There are internal assh library events to process
    process_assh_events(ssh);
  }

out:
  return;
}

// TODO: This isn't called from anywhere yet
void cleanup_session (struct ssh *ssh) {
  free(ssh->call_home_remote_address);
  free(ssh->event);
  XML_ParserFree(ssh->banner_parser);
  XML_ParserFree(ssh->hello_parser);
  XML_ParserFree(ssh->message_parser);
  assh_session_release(ssh->session);
  fclose(ssh->debug_file);
  if (ssh->calix_remote_ip != NULL) {
    free(ssh->calix_remote_ip);
  }
  free(ssh);
}

char* generate_random_filename(int length) {
  // debug_asdfadsf.txt
  //int length = 16;
  char* random_string = (char*)malloc(sizeof(char) * length+11);

  if (random_string == NULL) return NULL;

  memcpy(random_string, "debug_", 6);
  for (int i = 0; i < length; i++) {
    random_string[i+6] = (char)(rand() % 26 + 97);
  }

  memcpy(random_string+length+6, ".txt", 4);
  random_string[length+10] = '\0';

  return random_string;
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

  client_sd = accept(watcher->fd, (struct sockaddr *)&client_addr, &client_len);

  if (client_sd < 0) {
    perror("accept error");
    return;
  }

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
  
  struct ssh *ssh;
  ssh = malloc(sizeof(struct ssh));
  ssh->session = session;
  ssh->inter = inter;
  ssh->auth_methods = auth_methods;
  ssh->call_home_remote_address = call_home_remote_address;
  ssh->event = malloc(sizeof(struct assh_event_s));
  if (assh_event_get(ssh->session, ssh->event, time(NULL)) == 0) {
    // Bail
    // TODO: clean up properly
    abort();
  }
  // TODO: Add error checking
  ssh->banner_parser = XML_ParserCreate(NULL);
  XML_SetElementHandler(ssh->banner_parser, banner_start_element, banner_end_element);
  XML_SetCharacterDataHandler(ssh->banner_parser, banner_char_handler);
  XML_SetUserData(ssh->banner_parser, (void *)ssh);
  ssh->banner_is_complete = 0;
  ssh->banner_ack_is_complete = 0;
  ssh->hello_parser = XML_ParserCreate(NULL);
  XML_SetElementHandler(ssh->hello_parser, hello_start_element, hello_end_element);
  XML_SetCharacterDataHandler(ssh->hello_parser, hello_char_handler);
  XML_SetUserData(ssh->hello_parser, (void *)ssh);
  ssh->hello_end_tag_seen = 0;
  ssh->hello_is_complete = 0;
  ssh->hello_et_al_sent = 0;
  ssh->message_parser = XML_ParserCreate(NULL);
  XML_SetElementHandler(ssh->message_parser, message_start_element, message_end_element);
  XML_SetCharacterDataHandler(ssh->message_parser, message_char_handler);
  XML_SetUserData(ssh->message_parser, (void *)ssh);
  ssh->incoming_message_is_complete = 0;
  ssh->outgoing_message_is_complete = 0;
  ssh->server_type = NETCONF_SERVER_TYPE_CALIX;
  //ssh->message_tag_type = XML_TAG_TYPE_UNKNOWN;
  //ssh->xml_namespace = XML_NAMESPACE_UNKNOWN;
  //ssh->tag_stack = XML_TAG_STACK_NONE;
  ssh->tag_stack_head = malloc(sizeof(struct tag_stack_tailq_head));
  TAILQ_INIT(ssh->tag_stack_head);
  ssh->rpc_reply_message_id = 0;
  ssh->xml_char_buffer = NULL;
  ssh->xml_char_buffer_len = 0;
  ssh->calix_storage_object = CALIX_STORE_NONE;
  ssh->storage_struct = NULL;
  ssh->calix_remote_ip = NULL;
  //ssh->juniper_storage_object = 0;
  ssh->debug_file = fopen(generate_random_filename(16), "a");
  if (ssh->debug_file == NULL) {
    perror("Error opening debug file");
    abort();
  }

  //printf("Initial event state is %d\n", ssh->event->id);
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
}

static void timeout_cb (EV_P_ ev_timer *w, int revents) {
  fprintf (stderr, "timeout\n");
}

ev_signal signal_watcher;
static void sigint_cb (struct ev_loop *loop, ev_signal *w, int revents) {
  puts("catch SIGINT");
  ev_break (EV_A_ EVBREAK_ALL);
  const int enable = 1;
  // TODO: Remove this as it probably doesn't do what I think it does
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

//  struct tag_stack_tailq *tag = malloc(sizeof(*tag));
//  tag->tag_type = XML_TAG_TYPE_NONE;
//  TAILQ_INSERT_TAIL(tag_stack_head, tag, tags);

//  token = malloc(sizeof(*token));
//  token->token_id = ATTRIBUTENAME;
//  token->charval = strdup(atts[i]);
//  TAILQ_INSERT_TAIL(token_head, token, tokens);
//
//  token = malloc(sizeof(*token));
//  token->token_id = ATTRIBUTEVALUE;
//  token->charval = strdup(atts[i+1]);
//  TAILQ_INSERT_TAIL(token_head, token, tokens);
//
//  token = malloc(sizeof(*token));
//  token->token_id = TAGEND;
//  TAILQ_INSERT_TAIL(token_head, token, tokens);
//
//  struct token_tailq *token_item;
//  token_item = TAILQ_FIRST(token_head);
//  if (token_item != NULL) {
//    yylval.charval = token_item->charval;
//    ret = token_item->token_id;
//    TAILQ_REMOVE(token_head, token_item, tokens);
//    free(token_item);
//  }

  ev_run (loop, 0);

  abort();

  printf("Connection closed\n");

  // TODO: DO proper cleanup here
  //assh_session_release(session);
}
