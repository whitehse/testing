#!/bin/bash

( cd slapd ; ./destroy.sh )
( cd routers ; ./destroy.sh )
( cd switches ; ./destroy.sh )
#common/utils/set-simple-firewall-on-host.sh eth0
