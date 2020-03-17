#!/bin/bash

## Experimental
#
#TEMPDIR=`mktemp -d`
#
## Create an EC parameters file
#openssl ecparam -name prime256v1 -out "$TEMPDIR"/prime256v1.pem
#
## Display the data in the parameters file
#openssl ecparam -in "$TEMPDIR"/prime256v1.pem -text -noout
##ASN1 OID: prime256v1
##NIST CURVE: P-256
#
## And with
##echo ""
##openssl ecparam -in "$TEMPDIR"/prime256v1.pem -text -param_enc explicit -noout
#
## Generate key
#openssl ecparam -in "$TEMPDIR"/prime256v1.pem -genkey -noout -out "$TEMPDIR"/ca.key.pem
#
#rm "$TEMPDIR"/*
#rmdir "$TEMPDIR"
#
#exit

if [ -z "$2" ]; then
  cat - << EOF
Must provide two options:
 ./create-root-cert.sh <output_directory> <subject>

For example:
 ./create-root-cert.sh ca/ "/C=US/ST=OK/L=Bixby/O=Example Ltd./CN=Root CA"
EOF
  exit 1
fi

export DIR="$1"
export SUBJECT="$2"

if [ ! -d "$DIR" ]; then
  echo "$DIR must exist"
  exit 2
fi

if [ -d "$DIR/certs" ]; then
  echo "$DIR/certs exists, indicating an existing root CA. Exiting"
  exit 3
fi

mkdir -p "$DIR"/{certs,crl,newcerts,private}

CUR_DIR=`pwd`
cd "$DIR"

chmod 700 private
touch ./index.txt
openssl rand -hex 16 > serial
#openssl genrsa -out private/ca.key.pem 4096
#openssl genpkey -algorithm x25519 -out private/ca.key.pem
openssl ecparam -name prime256v1 -out ./prime256v1.pem
openssl ecparam -in ./prime256v1.pem -genkey -noout -out private/ca.key.pem
chmod 400 private/ca.key.pem
openssl req -config "$CUR_DIR"/root-config.txt \
      -key private/ca.key.pem \
      -new -x509 -days 7300 -sha256 -extensions v3_ca \
      -out certs/ca.cert.pem \
	  -subj "$SUBJECT"
chmod 444 certs/ca.cert.pem
