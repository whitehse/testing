#!/bin/bash

if [ -z "$2" ]; then
  cat - << EOF
Must provide two options:
 ./create-intermediate-cert.sh <existing_ca_directory> <subject>

For example:
 ./create-intermediate-cert.sh ca/ "/C=US/ST=OK/L=Bixby/O=Example Ltd./CN=Intermediate CA 1"
EOF
  exit 1
fi

export DIR="$1"
export SUBJECT="$2"

if [ ! -d "$DIR" ]; then
  echo "$DIR must exist and contain an existing root CA directory hierarchy. Exiting."
  exit 2
fi

if [ ! -d "$DIR/certs" ]; then
  echo "$DIR must point to an existing root CA directory. $DIR/certs does not exist. Exiting."
  exit 3
fi

if [ -d "$DIR/intermediate/certs" ]; then
  echo "$DIR/intermediate/certs already exists, indicating an existing intermediate certificate. Exiting."
  exit 4
fi

mkdir -p "$DIR"/intermediate/certs
mkdir -p "$DIR"/intermediate/crl
mkdir -p "$DIR"/intermediate/newcerts
mkdir -p "$DIR"/intermediate/private
mkdir -p "$DIR"/intermediate/csr

CUR_DIR=`pwd`

cd "$DIR"
chmod 700 intermediate/private
touch intermediate/index.txt
echo 1000 > intermediate/serial
echo 1000 > intermediate/crlnumber
openssl genrsa \
      -out intermediate/private/intermediate.key.pem 4096
chmod 400 intermediate/private/intermediate.key.pem
openssl req -config "$CUR_DIR"/intermediate-config.txt -new -sha256 \
      -key intermediate/private/intermediate.key.pem \
      -out intermediate/csr/intermediate.csr.pem \
	  -subj "$SUBJECT"
echo -n 'y
y
' | openssl ca -config "$CUR_DIR"/root-config.txt -extensions v3_intermediate_ca \
      -days 3650 -notext -md sha256 \
      -in intermediate/csr/intermediate.csr.pem \
      -out intermediate/certs/intermediate.cert.pem
chmod 444 intermediate/certs/intermediate.cert.pem
cat intermediate/certs/intermediate.cert.pem \
      certs/ca.cert.pem > intermediate/certs/ca-chain.cert.pem
chmod 444 intermediate/certs/ca-chain.cert.pem
#!/bin/bash

source bin/lib.sh

if [ -z "$1" ]; then
  cat - << EOF
A host must be specified.
EOF
  exit 1
fi

export HOST="$1"

ssh -F conf/ssh_config root@$HOST 'bash -s' << EOF
mkdir /root/ca
cd /root/ca
mkdir certs crl newcerts private
chmod 700 private
touch index.txt
echo 1000 > serial
wget -O /root/ca/openssl.cnf https://jamielinux.com/docs/openssl-certificate-authority/_downloads/root-config.txt
openssl genrsa -out private/ca.key.pem 4096
chmod 400 private/ca.key.pem
openssl req -config openssl.cnf \
      -key private/ca.key.pem \
      -new -x509 -days 7300 -sha256 -extensions v3_ca \
      -out certs/ca.cert.pem \
	  -subj "/C=US/ST=OK/L=Bixby/O=Example Ltd./CN=Root CA"
chmod 444 certs/ca.cert.pem
mkdir /root/ca/intermediate
cd /root/ca/intermediate
mkdir certs crl csr newcerts private
chmod 700 private
touch index.txt
echo 1000 > serial
echo 1000 > /root/ca/intermediate/crlnumber
wget -O /root/ca/intermediate/openssl.cnf https://jamielinux.com/docs/openssl-certificate-authority/_downloads/intermediate-config.txt
cd /root/ca
openssl genrsa \
      -out intermediate/private/intermediate.key.pem 4096
chmod 400 intermediate/private/intermediate.key.pem
openssl req -config intermediate/openssl.cnf -new -sha256 \
      -key intermediate/private/intermediate.key.pem \
      -out intermediate/csr/intermediate.csr.pem \
	  -subj "/C=US/ST=OK/L=Bixby/O=Example Ltd./CN=Intermediate CA"
echo -n 'y
y
' | openssl ca -config openssl.cnf -extensions v3_intermediate_ca \
      -days 3650 -notext -md sha256 \
      -in intermediate/csr/intermediate.csr.pem \
      -out intermediate/certs/intermediate.cert.pem
chmod 444 intermediate/certs/intermediate.cert.pem
cat intermediate/certs/intermediate.cert.pem \
      certs/ca.cert.pem > intermediate/certs/ca-chain.cert.pem
chmod 444 intermediate/certs/ca-chain.cert.pem

for foo in ldap27.example.com ldap27.example.org ldap26.example.com ldap26.example.org ldap23.example.com ldap23.example.org imap.example.com imap.example.org
do
  openssl genrsa \
    -out intermediate/private/\${foo}.key.pem 2048
  chmod 400 intermediate/private/\${foo}.key.pem
  openssl req -config intermediate/openssl.cnf \
    -key intermediate/private/\${foo}.key.pem \
    -new -sha256 -out intermediate/csr/\${foo}.csr.pem \
    -subj "/C=US/ST=OK/L=Bixby/O=Example Ltd./CN=\${foo}"
  echo -n 'y
y
' | openssl ca -config intermediate/openssl.cnf \
    -extensions server_cert -days 375 -notext -md sha256 \
    -in intermediate/csr/\${foo}.csr.pem \
    -out intermediate/certs/\${foo}.cert.pem
  openssl verify -CAfile intermediate/certs/ca-chain.cert.pem \
    intermediate/certs/\${foo}.cert.pem
done

openssl genrsa \
      -out intermediate/private/dwhite.example.com.key.pem 2048
chmod 400 intermediate/private/dwhite.example.com.key.pem
openssl req -config intermediate/openssl.cnf \
    -key intermediate/private/dwhite.example.com.key.pem \
    -new -sha256 -out intermediate/csr/dwhite.example.com.csr.pem \
    -subj "/C=US/ST=OK/L=Bixby/O=Example Ltd./CN=dwhite@example.com"
echo -n 'y
y
' | openssl ca -config intermediate/openssl.cnf \
      -extensions usr_cert -days 375 -notext -md sha256 \
      -in intermediate/csr/dwhite.example.com.csr.pem \
      -out intermediate/certs/dwhite.example.com.cert.pem
openssl verify -CAfile intermediate/certs/ca-chain.cert.pem \
      intermediate/certs/dwhite.example.com.cert.pem
EOF
