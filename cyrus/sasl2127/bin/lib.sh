#!/bin/bash

function verify_parameter_exists {
  if [ "x$1" == "x" ]; then
    cat - << EOF
Missing Parameter
EOF
    exit 1
  fi
}

function verify-package-installation {
  verify_parameter_exists "$1"
  dpkg -l "$1" > /dev/null
  if [ $? -ne 0 ]; then
    exit 1
  fi
}

function install-public-ssh-key {
  verify_parameter_exists "$1"
  MOUNT_POINT="$1"
  PUBLIC_KEY="`ssh-add -L`"
  if [ $? -ne 0 ]; then
    if [ -f ~/.ssh/id_rsa.pub ]; then
	  PUBLIC_KEY="`cat ~/.ssh/id_rsa.pub`"
	elif [ -f ~/.ssh/id_rsa.pub ]; then
	  PUBLIC_KEY="`cat ~/.ssh/id_rsa.pub`"
    else
	  cat - << EOF
No ssh public key found.
EOF
    fi
  fi
  if [ ! -d $MOUNT_POINT/root/.ssh ]; then
    sudo mkdir -p $MOUNT_POINT/root/.ssh
	sudo chmod 700 $MOUNT_POINT/root/.ssh
  fi
  export MOUNT_POINT
  export PUBLIC_KEY
  echo "$PUBLIC_KEY" | sudo tee --append "$MOUNT_POINT/root/.ssh/authorized_keys" > /dev/null
  sudo chmod 600 $MOUNT_POINT/root/.ssh/authorized_keys
}
