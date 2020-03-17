#!/bin/bash

NAME="$1"
SUBJECT="/C=US/ST=OK/L=Bixby/O=Example Ltd./CN=${NAME}"
#SUBJECT="$2"

#if [ -z "$2" ]; then
#  cat - << EOF
#Must provide two options:
# ./create-intermediate-cert.sh <existing_ca_directory> <subject>
#
#For example:
# ./create-intermediate-cert.sh ca/ "/C=US/ST=OK/L=Bixby/O=Example Ltd./CN=Intermediate CA 1"
#EOF
#  exit 1
#fi
#
#export DIR="$1"
#export SUBJECT="$2"
#
#if [ ! -d "$DIR" ]; then
#  echo "$DIR must exist and contain an existing root CA directory hierarchy. Exiting."
#  exit 2
#fi
#
#if [ ! -d "$DIR/certs" ]; then
#  echo "$DIR must point to an existing root CA directory. $DIR/certs does not exist. Exiting."
#  exit 3
#fi
#
#if [ -d "$DIR/intermediate/certs" ]; then
#  echo "$DIR/intermediate/certs already exists, indicating an existing intermediate certificate. Exiting."
#  exit 4
#fi
#
#mkdir -p "$DIR"/intermediate/{certs,crl,newcerts,private,csr}
#
#CUR_DIR=`pwd`
#
#cd "$DIR"
#chmod 700 intermediate/private
#touch intermediate/index.txt
#openssl rand -hex 16 > intermediate/serial
#openssl rand -hex 16 > intermediate/crlnumber
#
##openssl genrsa \
##      -out intermediate/private/intermediate.key.pem 4096
##openssl genpkey -algorithm x25519 \
##      -out intermediate/private/intermediate.key.pem
#openssl ecparam -name prime256v1 -out ./prime256v1.pem
#openssl ecparam -in ./prime256v1.pem \
#      -genkey -noout -out intermediate/private/intermediate.key.pem
#
#chmod 400 intermediate/private/intermediate.key.pem
#openssl req -config "$CUR_DIR"/intermediate-config.txt -new -sha256 \
#      -key intermediate/private/intermediate.key.pem \
#      -out intermediate/csr/intermediate.csr.pem \
#	  -subj "$SUBJECT"
#echo -n 'y
#y
#' | openssl ca -config "$CUR_DIR"/root-config.txt -extensions v3_intermediate_ca \
#      -days 3650 -notext -md sha256 \
#      -in intermediate/csr/intermediate.csr.pem \
#      -out intermediate/certs/intermediate.cert.pem
#chmod 444 intermediate/certs/intermediate.cert.pem
#cat intermediate/certs/intermediate.cert.pem \
#      certs/ca.cert.pem > intermediate/certs/ca-chain.cert.pem
#chmod 444 intermediate/certs/ca-chain.cert.pem

openssl ecparam -out ${NAME}-priv.key -name prime256v1 -genkey

openssl req -new -key ${NAME}-priv.key -out ${NAME}.csr -config "server-config.txt" -subj="${SUBJECT}"

#openssl ca -config server-config.txt \
#      -extensions server_cert -days 375 -notext -md sha256 \
#      -in intermediate/csr/www.example.com.csr.pem \
#      -out intermediate/certs/www.example.com.cert.pem
## chmod 444 intermediate/certs/www.example.com.cert.pem

