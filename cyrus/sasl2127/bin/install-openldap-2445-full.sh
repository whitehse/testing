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
cd /usr/src/
rm -rf openldap-2.4.45/
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
--enable-syslog \
--enable-proctitle \
--enable-ipv6 \
--enable-local \
--enable-slapd \
--enable-dynacl \
--enable-aci \
--enable-cleartext \
--enable-crypt \
--disable-lmpasswd \
--enable-spasswd \
--enable-modules \
--enable-rewrite \
--enable-rlookups \
--enable-slapi \
--enable-wrappers \
--enable-backends=mod \
--disable-ndb \
--enable-overlays=mod \
--with-cyrus-sasl \
--with-threads \
--with-tls=openssl \
--with-odbc=unixodbc
--enable-ldapdb \
--with-ldap=/usr
make depend && make && make install
ldconfig
EOF
