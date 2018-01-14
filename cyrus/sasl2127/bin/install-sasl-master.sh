#!/bin/bash

source bin/lib.sh

if [ -z "$1" ]; then
  cat - << EOF
A host must be specified.
EOF
  exit 1
fi

if [ -z "$2" ]; then
  cat - << EOF
Must specificy a kerberos library: mit,heimdal
  exit 1
EOF

export HOST="$1"
export LIB="$2"

ssh -F conf/ssh_config $HOST 'bash -l -s' << EOF
apt-get -y install unzip
apt-get -y install chrpath default-libmysqlclient-dev diffstat libdb-dev libmariadbclient-dev libmariadbclient-dev-compat libmariadbclient18 libpam0g-dev libsqlite3-dev mysql-common quilt
cd /usr/src
#git clone https://github.com/cyrusimap/cyrus-sasl.git
wget --no-check-certificate https://github.com/cyrusimap/cyrus-sasl/archive/master.zip
unzip master.zip
cd cyrus-sasl-master
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
--enable-gss_mutexes \
--with-gss_impl=$LIB
make && make install
ldconfig
EOF
