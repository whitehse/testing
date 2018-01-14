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
apt-get -y build-dep krb5-kdc
cd /usr/src
wget --no-check-certificate https://kerberos.org/dist/krb5/1.16/krb5-1.16.tar.gz
tar -xvzf krb5-1.16.tar.gz
cd krb5-1.16/src
./configure \
--prefix=/usr \
--localstatedir=/etc \
--mandir=/usr/share/man \
--with-system-et --with-system-ss --disable-rpath \
--enable-shared --with-ldap --without-tcl \
--with-system-verto \
--libdir=/usr/lib \
--sysconfdir=/etc
make && make install
ldconfig
EOF
