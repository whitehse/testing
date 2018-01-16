#!/bin/bash

source bin/lib.sh

if [ -z "$1" ]; then
  cat - << EOF
A host must be specified.
EOF
  exit 1
fi

HOST="$1"

ssh -F conf/ssh_config $HOST 'bash -l -s' << EOF
apt-get -y remove heimdel-dev
apt-get -y remove heimdal-kdc
apt-get -y remove heimdal-clients
EOF

for foo in krb5-user krb5-kdc krb5-kdc-ldap krb5-admin-server krb5-kpropd krb5-multidev libkrb5-dev libkrb5-dbg krb5-pkinit krb5-otp krb5-k5tls krb5-doc libkrb5-3 libgssapi-krb5-2 libgssrpc4 libkadm5srv-mit11 libkadm5clnt-mit11 libk5crypto3 libkdb5-8 libkrb5support0 libkrad0 krb5-gss-samples krb5-locales libkrad-dev
do
  bin/install-dummy-package.sh $HOST $foo
done
