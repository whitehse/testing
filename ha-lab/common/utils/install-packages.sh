#!/bin/bash

# $1 - container
# $2 - packages, separated by comma

CONTAINER="$1"
PACKAGES=`echo "$2" | sed 's/,/ /g'`

sudo lxc-attach -n $CONTAINER /bin/bash -l -s << EOF
apt-get -y install $PACKAGES
EOF

