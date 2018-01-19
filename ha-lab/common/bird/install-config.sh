#!/bin/bash

# $1 - Container
# $2 - File

CONTAINER="$1"
FILE="$2"

sudo lxc-attach -n $CONTAINER /bin/bash -l -s < $FILE
