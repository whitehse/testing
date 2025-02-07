#!/usr/bin/bash

gdbtui -ex="target remote | ssh -T map.ecolink.coop gdbserver - ./netconf"
