#!/usr/bin/bash

#gcc -I. -o netconf netconf.c -lev -lassh -lcbor -lcjson
./generate_netconf_gperf.pl > netconf_generated.h
gcc -I. -o netconf netconf.c -lev -lcbor -lcjson -lassh -lexpat -g
