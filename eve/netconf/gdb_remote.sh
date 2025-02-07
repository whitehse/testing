#!/usr/bin/bash

gdb -ex="target remote | ssh -T map.ecolink.coop gdbserver - ./netconf"
