#!/bin/bash

source bin/lib.sh

if [ -z "$1" ]; then
  cat - << EOF
A host must be specified.
EOF
  exit 1
fi

export HOST="$1"

# make and gcc are needed on the host
ssh -F conf/ssh_config $HOST 'bash -l -s' << EOF
apt-get -y install chrpath default-libmysqlclient-dev diffstat libdb-dev libmariadbclient-dev libmariadbclient-dev-compat libmariadbclient18 libpam0g-dev libsqlite3-dev mysql-common quilt
cd /usr/src
wget ftp://ftp.andrew.cmu.edu/pub/cyrus-mail/cyrus-sasl-2.1.23.tar.gz
cd cyrus-sasl-2.1.23
CFLAGS="-O0 -g" ./configure \
--prefix=/usr \
--mandir=/usr/share/man \
--infodir=/usr/share/info \
--enable-alwaystrue \
--enable-srp \
--enable-srp-setpass \
--enable-login \
--enable-ntlm \
--enable-passdss \
--enable-share \
--with-saslauthd=/var/run/saslauthd \
--with-authdaemond=/var/run/courier \
--sysconfdir=/etc \
--enable-httpform \
--with-devrandom=/dev/urandom \
--enable-gssapi \
--enable-gss_mutexes
make && make install
ldconfig
EOF
