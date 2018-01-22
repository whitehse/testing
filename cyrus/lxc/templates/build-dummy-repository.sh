#!/bin/bash

# Copy this file underneath /var/cache/lxc/dummy-packages
# or modify the template files to copy it from another source

mkdir /var/local/dummy-packages
cd /var/local/dummy-packages

for foo in libgnutls28-dev libgnutls30 gnutls-bin gnutls-doc libgnutlsxx28 libgnutls-openssl27 libgnutls-dane0
do
  apt-cache show "$foo" | grep -v Depends > control
  equivs-build control
  rm control
done

for foo in heimdal-docs heimdal-kdc heimdal-multidev heimdal-dev heimdal-clients heimdal-kcm heimdal-servers heimdal-dbg libheimbase1-heimdal libasn1-8-heimdal libkrb5-26-heimdal libhdb9-heimdal libkadm5srv8-heimdal libkadm5clnt7-heimdal libgssapi3-heimdal libkafs0-heimdal libroken18-heimdal libotp0-heimdal libsl0-heimdal libkdc2-heimdal libhx509-5-heimdal libheimntlm0-heimdal libwind0-heimdal libhcrypto4-heimdal
do
  apt-cache show "$foo" | grep -v Depends > control
  equivs-build control
  rm control
done

for foo in sasl2-bin cyrus-sasl2-doc libsasl2-2 libsasl2-modules libsasl2-modules-db libsasl2-modules-ldap libsasl2-modules-otp libsasl2-modules-sql libsasl2-modules-gssapi-mit libsasl2-dev libsasl2-modules-gssapi-heimdal
do
  apt-cache show "$foo" | grep -v Depends > control
  equivs-build control
  rm control
done

for foo in krb5-user krb5-kdc krb5-kdc-ldap krb5-admin-server krb5-kpropd krb5-multidev libkrb5-dev libkrb5-dbg krb5-pkinit krb5-otp krb5-k5tls krb5-doc libkrb5-3 libgssapi-krb5-2 libgssrpc4 libkadm5srv-mit11 libkadm5clnt-mit11 libk5crypto3 libkdb5-8 libkrb5support0 libkrad0 krb5-gss-samples krb5-locales libkrad-dev
do
  apt-cache show "$foo" | grep -v Depends > control
  equivs-build control
  rm control
done

for foo in slapd slapd-smbk5pwd ldap-utils libldap-2.4-2 libldap-common libldap2-dev
do
  apt-cache show "$foo" | grep -v Depends > control
  equivs-build control
  rm control
done

for foo in openssl libssl1.1 libssl-dev libssl-doc libssl1.0.2 libssl1.0-dev
do
  apt-cache show "$foo" | grep -v Depends > control
  equivs-build control
  rm control
done

for foo in cyrus-common cyrus-doc cyrus-imapd cyrus-pop3d cyrus-admin cyrus-murder cyrus-replication cyrus-nntpd cyrus-caldav cyrus-clients cyrus-dev libcyrus-imap-perl
do
  apt-cache show "$foo" | grep -v Depends > control
  equivs-build control
  rm control
done

dpkg-scanpackages . /dev/null > Packages
gzip --keep --force -9 Packages

cat > Release << EOF
Archive: archive
Component: component
Origin: Example
Label: Example
Architecture: all
EOF

echo -e "Date: `LANG=C date -Ru`" >> Release
echo -e 'MD5Sum:' >> Release
printf ' '$(md5sum Packages.gz | cut --delimiter=' ' --fields=1)' %16d Packages.gz' $(wc --bytes Packages.gz | cut --delimiter=' ' --fields=1) >> Release
printf '\n '$(md5sum Packages | cut --delimiter=' ' --fields=1)' %16d Packages' $(wc --bytes Packages | cut --delimiter=' ' --fields=1) >> Release
echo -e '\nSHA256:' >> Release
printf ' '$(sha256sum Packages.gz | cut --delimiter=' ' --fields=1)' %16d Packages.gz' $(wc --bytes Packages.gz | cut --delimiter=' ' --fields=1) >> Release
printf '\n '$(sha256sum Packages | cut --delimiter=' ' --fields=1)' %16d Packages' $(wc --bytes Packages | cut --delimiter=' ' --fields=1) >> Release
