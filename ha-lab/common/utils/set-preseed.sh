#!/bin/bash

CONTAINER="$1"
PRESEED="$2"

sudo lxc-attach -n $CONTAINER /bin/bash -l -s < $PRESEED
