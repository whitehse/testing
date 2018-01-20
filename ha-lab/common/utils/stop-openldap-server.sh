#!/bin/bash

# $1 - Container

CONTAINER="$1"

sudo lxc-attach -n $CONTAINER /bin/bash -l -s << EOF
service slapd stop
EOF
