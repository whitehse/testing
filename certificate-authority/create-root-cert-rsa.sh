#!/bin/bash

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
touch index.txt
echo 1000 > serial
openssl genrsa -out private/ca.key.pem 4096
chmod 400 private/ca.key.pem
openssl req -config "$CUR_DIR"/root-config.txt \
      -key private/ca.key.pem \
      -new -x509 -days 7300 -sha256 -extensions v3_ca \
      -out certs/ca.cert.pem \
	  -subj "$SUBJECT"
chmod 444 certs/ca.cert.pem
