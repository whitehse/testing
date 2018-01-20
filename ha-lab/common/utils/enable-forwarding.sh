#!/bin/bash

# $1 - container

CONTAINER="$1"

sudo lxc-attach -n $CONTAINER /bin/bash -l -s << EOF
sed -i 's/^#net\.ipv4\.ip_forward.*/net.ipv4.ip_forward=1/' /etc/sysctl.conf
sysctl -p
EOF

