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
apt-get -y install cm-super-minimal dh-systemd docutils-common doxygen doxygen-latex fontconfig-config fonts-dejavu-core fonts-lmodern ghostscript libavahi-client3 libavahi-common-data libavahi-common3 libcairo2 libclang1-3.9 libcups2 libcupsimage2 libdbus-1-3 libev4 libfontconfig1 libfreetype6 libgraphite2-3 libgs9 libgs9-common libharfbuzz-icu0 libharfbuzz0b libijs-0.35 libjbig0 libjbig2dec0 libjpeg62-turbo libjs-jquery libjs-sphinxdoc libjs-underscore libkeyutils-dev libkpathsea6 liblcms2-2 libldap2-dev libllvm3.9 libnspr4 libnss3 libopenjp2-7 libpaper-utils libpaper1 libpixman-1-0 libpng16-16 libpoppler64 libpotrace0 libptexenc1 libpython-stdlib libpython2.7-minimal libpython2.7-stdlib libsynctex1 libtexlua52 libtexluajit2 libtiff5 libverto-dev libverto-glib1 libverto-libev1 libverto1 libxapian30 libxaw7 libxcb-render0 libxcb-shm0 libxext6 libxi6 libxmu6 libxpm4 libxrender1 libxslt1.1 libzzip-0-13 poppler-data preview-latex-style python python-alabaster python-babel python-babel-localedata python-cheetah python-docutils python-imagesize python-jinja2 python-lxml python-markupsafe python-minimal python-pkg-resources python-pygments python-roman python-six python-sphinx python-tz python2.7 python2.7-minimal sphinx-common t1utils texlive-base texlive-binaries texlive-extra-utils texlive-font-utils texlive-fonts-recommended texlive-generic-extra texlive-latex-base texlive-latex-extra texlive-latex-recommended texlive-pictures xdg-utils
cd /usr/src
wget https://kerberos.org/dist/krb5/1.16/krb5-1.16.tar.gz
cd krb5-1.15/src
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
