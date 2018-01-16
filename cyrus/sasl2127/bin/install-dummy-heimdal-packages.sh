#!/bin/bash

source bin/lib.sh

if [ -z "$1" ]; then
  cat - << EOF
A host must be specified.
EOF
  exit 1
fi

ssh -F conf/ssh_config $HOST 'bash -l -s' << EOF
apt-get -y remove krb5-dev
EOF

for foo in heimdal-docs heimdal-kdc heimdal-multidev heimdal-dev heimdal-clients heimdal-kcm heimdal-servers heimdal-dbg libheimbase1-heimdal libasn1-8-heimdal libkrb5-26-heimdal libhdb9-heimdal libkadm5srv8-heimdal libkadm5clnt7-heimdal libgssapi3-heimdal libkafs0-heimdal libroken18-heimdal libotp0-heimdal libsl0-heimdal libkdc2-heimdal libhx509-5-heimdal libheimntlm0-heimdal libwind0-heimdal libhcrypto4-heimdal
do
  bin/install-dummy-package.sh $1 $foo
done
