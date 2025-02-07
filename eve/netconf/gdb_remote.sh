#!/usr/bin/bash

# cat >> ~/gdbinit <EOF
# set sysroot /
# #set substitute-path /usr/src/include /mnt/include
# EOF 
gdbtui -ex="target remote | ssh -T map.ecolink.coop gdbserver - ./netconf" ./netconf
