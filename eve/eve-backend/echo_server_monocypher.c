#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

#include <ev.h>
//#define EV_STANDALONE 1
//#include <ev.c>
#include <sodium.h>
#include <monocypher.h>
#include <cfgpath.h>

uint8_t shared_keys[64]; /* Two shared session keys */
const uint8_t *client_initial_key;
const uint8_t *server_initial_key;

#define EVE_CONNECTION_INIT        1
#define EVE_CONNECTION_ESTABLISHED 2
struct _connection {
  uint64_t client_id;
  uint8_t state;
  uint8_t client_k1[32];
  uint8_t client_k2[32];
  uint8_t server_k1[32];
  uint8_t server_k2[32];
};
typedef struct _connection connection_t;

void hex_dump(char *desc, void *addr, int len) {
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

#define BUFFER_SIZE 1024

void echo_read_cb(struct ev_loop *loop, struct ev_io *w, int revents){
  char buffer[BUFFER_SIZE];
  //char buffer[1024];
  ssize_t read;

  connection_t *connection = w->data;

  //hex_dump(NULL, client_initial_key, 32);

  if(EV_ERROR & revents) {
    perror("got invalid event");
    return;
  }

  memset(buffer, 0, sizeof(buffer));
  //int size = read(w->fd, buffer, sizeof(buffer));
  read = recv(w->fd, buffer, BUFFER_SIZE, 0);
  if ( read < 0 ) {
    perror("read error");
    exit(5);
  }

  if (read == 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      perror("Something weird happened");
      return 0;
    } else {
      puts("Socket was closed");
      ev_io_stop(loop, w);
      close(w->fd);
      free(w);
      return 0;
    }
  }

  printf("received header, or %d bytes from client\n", read);
  //hex_dump(NULL, buffer, 120);

  // Received from the client
  //
  // First order of business is to create session keys. The first segment of
  // data (PDU) sent to the server will contain:
  //   uint64_t    - unique client identifier (in clear text)
  //   uint8_t[24] - nonce (in clear text)
  //   uint8_t[32] - k1, the future chacha20 client session key used to encrypt
  //                 the 4 byte length of successive PDUs
  //   uint8_t[32] - k2, the future client session key used in conjuction with
  //                 poly1305 to contruct the AEAD of future PDUs
  //   uint8_t[32] - authentication cookie
  //   uint8_t[16] - message authentication code
  //
  // The total length of the initial PDU, including the client identifier, is
  // 120 bytes, by protocol, and does not use a length field. The appended MAC
  //
  // After authentication, the server will generate its own, separate, k1 and k2
  //
  // Note that the first four bytes, encrypted with k1, will become the Additional
  // Data of the AEAD PDU, of which the cyphertext portion will be encrypted with
  // k2. The encrypted length data will, eventually, be authenticated. However it
  // will first be decrypted, without authentication, to determine the length of
  // the cyphertext of the PDU. There is room for foul play should an attacker
  // corrupt the length. For example, the corruption could indicate a
  // very large PDU size, leading to eventual buffer exhaustion. A reasonable
  // upper limit should be set for the maximum length of a PDU, by protocol. The
  // MAC, transmitted after the cyphertext, will need to be in possession for the
  // entire PDU to be authenticated.
  //
  // The keys, k1 and k2, will be regenerated, or rotated, after each 1 gigabyte
  // of data transferred. This is independently calculated by the client and the
  // the server. That is, key rotation will occur asymmetrically and at different
  // times between the two directions of the connection.
  //
  // The authentication cookie is generated by the server and supplied by the
  // client on each authenticated connection. This will be used, by the server,
  // to control how often client/password authentication and/or TOTP multi-factor
  // authentication will be entered by the user. That is, if the authentication
  // cookie is, say, more than one week old, the server may require a new
  // multi-factor authentication code. Authentication policy will be
  // determined by the server system administrator. The client should send
  // an authentication cookie consisting of all zeros (32 bytes) on the first
  // authentication to the server, or should the client wish to reset
  // authentication state.
  //
  // The nonce will equal the number of cyphertext bytes transferred, and will
  // hence be unique for each AEAD PDU generated, per session key. It is used
  // to track when the 1 gigabyte threshold has been crossed and when key rotation
  // should occur. It is reset to zero each time the key is regenerated/rotated
  // (to be used in the subsequent PDU sent).
  //
  // However, the nonce will be randomly generated for the initial client
  // and server key rotation PDUs, which use the X25519 and blake2b
  // deterministically generated keys. Otherwise, a nonce of zero would be
  // reused for those keys each time a connection is established, leading to
  // doom.

  //uint8_t initial_client_nonce  [24]; /* randomly generated */
  uint8_t initial_nonce [24]; /* randomly generated */
  uint8_t client_nonce  [24]; /* Use once per key, equal to bytes transferred */
  uint8_t nonce         [24]; /* Use once per key, equal to bytes transferred */
  uint8_t client_mac    [16]; /* Message authentication code */  
  uint8_t mac           [16]; /* Message authentication code */  
  uint8_t client_cookie [32];

  // Generate new session keys from random data
  getrandom(connection->server_k1, 32, 0);
  getrandom(connection->server_k2, 32, 0);

  // Generate random nonce for initial PDU to client, which rotates the key(s)
  //getrandom(initial_nonce, 24, 0);
  memset(initial_nonce, 0, 24);
 
  // Zero out the nonce
  //memset(nonce, 0, 24);

  // Zero out cookie 
  //memset(cookie, 1, 32);

  // Create a 64-bit counter that points to the first 8 bytes of the nonce.
  // The last 16 bytes will always be zero.
  // Endianness of this value is whatever the local host uses.
  uint64_t *nonce_counter = (uint64_t *)&nonce;

  // Decrypt the payload
  //int r = crypto_aead_unlock(buffer+32, buffer+128,
  //               client_initial_key, buffer+8,
  //               buffer, 8,
  //               buffer+32, 96);

  int crypto_aead_xchacha20poly1305_ietf_decrypt_detached(
    unsigned char *m,
    unsigned char *nsec,
    const unsigned char *c,
    unsigned long long clen,
    const unsigned char *mac,
    const unsigned char *ad,
    unsigned long long adlen,
    const unsigned char *npub,
    const unsigned char *k) {
    /* message forged! */
  }

  printf("crypto_aead_unlock returned %d\n", r);
  if (r != -1) {
    hex_dump(NULL, (void *) &buffer, 120);
  }

  //uint64_t client_id;
  memcpy(&connection->client_id, buffer, sizeof(uint64_t));
  connection->client_id = be64toh(connection->client_id);
  printf("Client ID = %ju\n", connection->client_id);
  //memcpy(buffer_initial+104, mac, 16);

  memcpy(connection->client_k1, buffer+32, 32);
  memcpy(connection->client_k2, buffer+64, 32);

  hex_dump("client_k1", connection->client_k1, 32);
  hex_dump("client_k2", connection->client_k2, 32);
}

void echo_accept_cb(struct ev_loop *loop, struct ev_io *w, int revents) {
  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);
  int client_sd;
  struct ev_io *w_client = (struct ev_io*) malloc (sizeof(struct ev_io));

  if(EV_ERROR & revents) {
    perror("got invalid event");
    return;
  }

  client_sd = accept(w->fd, (struct sockaddr *)&client_addr, &client_len);

  if (client_sd < 0) {
    perror("accept error");
    return;
  }

  printf("Successfully connected with client.\n");

  connection_t *connection = malloc(sizeof(connection_t));
  memset(connection, 0, sizeof(connection_t));
  connection->state = EVE_CONNECTION_INIT;
  w_client->data = connection;

  ev_io_init(w_client, echo_read_cb, client_sd, EV_READ);
  ev_io_start(loop, w_client);
}

int main(int argc, char const *argv[]) {
  struct ev_loop *loop = ev_default_loop(0);
  FILE *fptr;
  const uint8_t their_pk     [32]; /* Their public key          */
  uint8_t       your_sk      [32]; /* Your secret key           */
  uint8_t       your_pk      [32]; /* Your public key           */
  uint8_t       shared_secret[32]; /* Shared secret (NOT a key) */

  // server public key
  if ((fptr = fopen("server.pk","rb")) == NULL){
    printf("Error! opening file");
    exit(1);
  }
  fread(your_pk, 32, 1, fptr);
  fclose(fptr); 

  // server secret key
  if ((fptr = fopen("server.sk","rb")) == NULL){
    printf("Error! opening file");
    exit(1);
  }
  fread(&your_sk, 32, 1, fptr);
  fclose(fptr); 

  // client public key
  if ((fptr = fopen("client.pk","rb")) == NULL){
    printf("Error! opening file");
    exit(1);
  }
  fread(&their_pk, 32, 1, fptr);
  fclose(fptr); 

  if (sodium_init() == -1) {
    return 1;
  }

/*
  crypto_x25519(shared_secret, your_sk, their_pk);
  crypto_wipe(your_sk, 32);

  crypto_blake2b_ctx ctx;
  crypto_blake2b_init  (&ctx, 64);
  crypto_blake2b_update(&ctx, shared_secret, 32);
  crypto_blake2b_update(&ctx, their_pk     , 32);
  crypto_blake2b_update(&ctx, your_pk      , 32);
  crypto_blake2b_final (&ctx, shared_keys);
  hex_dump(NULL, shared_keys, 64);
  client_initial_key = shared_keys;
  server_initial_key = shared_keys + 32;
*/
  client_initial_key = malloc(crypto_kx_SESSIONKEYBYTES);
  server_initial_key = malloc(crypto_kx_SESSIONKEYBYTES);
  if (crypto_kx_server_session_keys(server_initial_key, client_initial_key,
                                  your_pk, your_sk, their_pk) != 0) {
    puts("crypto_kx_server_session_keys failed");
  }

  int echo_server_fd;
  struct sockaddr_in server, client;
  int len;
  int echo_port = 1235;
  echo_server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (echo_server_fd < 0) {
    perror("Cannot create socket");
    exit(1);
  }
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(echo_port);
  len = sizeof(server);
  if (bind(echo_server_fd, (struct sockaddr *)&server, len) < 0) {
    perror("Cannot bind sokcet");
    exit(2);
  }
  if (listen(echo_server_fd, 100000) < 0) {
    perror("Listen error");
    exit(3);
  }

  struct ev_io *w_accept = (struct ev_io*) malloc (sizeof(struct ev_io));
  ev_io_init(w_accept, echo_accept_cb, echo_server_fd, EV_READ);
  ev_io_start(loop, w_accept);

  /* Wipe secrets if they are no longer needed */
  crypto_wipe(shared_secret, 32);

  ev_run (loop, 0);

  return 0;
}
