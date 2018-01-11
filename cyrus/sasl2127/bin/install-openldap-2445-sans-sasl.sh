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
apt-get -y install autoconf automake autopoint autotools-dev comerr-dev debhelper dh-autoreconf dh-strip-nondeterminism gettext intltool-debian libarchive-zip-perl libcroco3 libdb5.3-dev libfile-stripnondeterminism-perl libglib2.0-0 libgmp-dev libgmpxx4ldbl libgnutls-dane0 libidn11-dev libltdl-dev libltdl7 libodbc1 libp11-kit-dev libperl-dev libsigsegv2 libtasn1-6-dev libtool libunbound2 libwrap0-dev m4 nettle-dev odbcinst odbcinst1debian2 pkg-config po-debconf time unixodbc-dev zlib1g-dev libssl-dev
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
