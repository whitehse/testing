#!/usr/bin/bash

# cat >> ~/gdbinit <EOF
# set sysroot /
# EOF 
gdbtui -ex="target remote | ssh -T map.ecolink.coop gdbserver - ./netconf" ./netconf
