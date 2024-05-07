#include <stdio.h>
#include <stdlib.h>
#include <sodium.h>

int main() {
  int n;
  FILE *fptr;

  /*
  unsigned char server_pk[crypto_sign_PUBLICKEYBYTES];
  unsigned char server_sk[crypto_sign_SECRETKEYBYTES];
  crypto_sign_keypair(server_pk, server_sk);
  */

  unsigned char server_pk[crypto_kx_PUBLICKEYBYTES], server_sk[crypto_kx_SECRETKEYBYTES];
  unsigned char server_rx[crypto_kx_SESSIONKEYBYTES], server_tx[crypto_kx_SESSIONKEYBYTES];
  /* Generate the client's key pair */
  crypto_kx_keypair(server_pk, server_sk);

  if ((fptr = fopen("server.pk","wb")) == NULL){
    printf("Error! opening file");

    // Program exits if the file pointer returns NULL.
    exit(1);
  }

  fwrite(&server_pk, crypto_kx_PUBLICKEYBYTES, 1, fptr); 
  fclose(fptr); 

  if ((fptr = fopen("server.sk","wb")) == NULL){
    printf("Error! opening file");

    // Program exits if the file pointer returns NULL.
    exit(1);
  }

  fwrite(&server_sk, crypto_kx_SECRETKEYBYTES, 1, fptr); 
  fclose(fptr); 

  unsigned char client_pk[crypto_kx_PUBLICKEYBYTES], client_sk[crypto_kx_SECRETKEYBYTES];
  unsigned char client_rx[crypto_kx_SESSIONKEYBYTES], client_tx[crypto_kx_SESSIONKEYBYTES];
  /* Generate the client's key pair */
  crypto_kx_keypair(client_pk, client_sk);

  if ((fptr = fopen("client.pk","wb")) == NULL){
    printf("Error! opening file");

    // Program exits if the file pointer returns NULL.
    exit(1);
  }

  fwrite(&client_pk, crypto_kx_PUBLICKEYBYTES, 1, fptr); 
  fclose(fptr); 

  if ((fptr = fopen("client.sk","wb")) == NULL){
    printf("Error! opening file");

    // Program exits if the file pointer returns NULL.
    exit(1);
  }

  fwrite(&client_sk, crypto_kx_SECRETKEYBYTES, 1, fptr); 
  fclose(fptr); 

  return 0;
}
