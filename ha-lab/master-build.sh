#!/bin/bash

dpkg -l bird > /dev/null
if [ $? -ne 0 ]; then
  sudo apt-get -y bird
  sudo cp -a host-bird-config/* /etc/bird/
  sudo chown -R bird:bird /etc/bird/
fi

# eth0 should be replaced with your publicly facing interface:w
common/utils/set-simple-firewall-on-host.sh eth0

( cd switches ; ./build.sh )
( cd routers ; ./build.sh )
( cd slapd ; ./build.sh )
