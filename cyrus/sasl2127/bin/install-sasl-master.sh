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
ssh -F conf/ssh_config root@$HOST 'bash -s' << EOF
apt-get -y install chrpath default-libmysqlclient-dev diffstat docbook docbook-to-man libdb-dev libmariadbclient-dev libmariadbclient-dev-compat libmariadbclient18 libosp5 libpam0g-dev libsqlite3-dev mysql-common opensp quilt sgml-data
cd /usr/src
git clone https://github.com/cyrusimap/cyrus-sasl.git
cd cyrus-sasl
ls
CFLAGS="-O0 -g" ./autogen.sh \
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
