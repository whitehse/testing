#!/bin/bash

source bin/lib.sh

if [ -z "$1" ]; then
  cat - << EOF
A host must be specified.
EOF
  exit 1
fi

export HOST="$1"

ssh -F conf/ssh_config $HOST 'bash -l -s' << EOF
apt-get -y build-dep slapd
cd /usr/src/
wget ftp://ftp.openldap.org/pub/OpenLDAP/openldap-release/openldap-2.4.45.tgz
tar -xvzf openldap-2.4.45.tgz
cd openldap-2.4.45/
CFLAGS="-O0 -g" ./configure \
--prefix=/usr \
--libexecdir=/usr/lib \
--sysconfdir=/etc \
--localstatedir=/var \
--mandir=/usr/share/man \
--enable-dynamic \
--enable-slapd=no \
--enable-crypt=yes \
--enable-spasswd=no \
--disable-backends \
--enable-overlays=no \
--with-cyrus-sasl=no \
--with-threads \
--with-tls=openssl
make depend && make && make install
ldconfig
EOF
