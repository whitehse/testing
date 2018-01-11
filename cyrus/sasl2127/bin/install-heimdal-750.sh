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
apt-get -y install bison flex libbison-dev libbsd-dev libcap-ng-dev libdb-dev libedit-dev libhesiod-dev libhesiod0 libice-dev libice6 libjson-perl libncurses5-dev libperl4-corelibs-perl libpthread-stubs0-dev libsm-dev libsm6 libsqlite3-dev libtext-unidecode-perl libtinfo-dev libx11-dev libxau-dev libxcb1-dev libxdmcp-dev libxml-libxml-perl libxml-namespacesupport-perl libxml-sax-base-perl libxml-sax-perl libxt-dev libxt6 ss-dev tex-common texinfo unzip x11-common x11proto-core-dev x11proto-input-dev x11proto-kb-dev xorg-sgml-doctools xtrans-dev
cd /usr/src
wget https://github.com/heimdal/heimdal/releases/download/heimdal-7.5.0/heimdal-7.5.0.tar.gz
tar -xvzf heimdal-7.5.0.tar.gz
cd heimdal-7.5.0
CFLAGS=-O0 ./configure \
--prefix=/usr \
--libexecdir=/usr/sbin \
--enable-shared \
--includedir=/usr/include \
--with-openldap=/usr \
--with-sqlite3=/usr \
--with-libedit=/usr \
--enable-kcm \
--with-hdbdir=/var/lib/heimdal-kdc \
--with-openssl=/usr \
--infodir=/usr/share/info \
--datarootdir=/usr/share \
--libdir=/usr/lib \
--without-krb4
make && make install
ldconfig
EOF
