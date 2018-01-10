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
