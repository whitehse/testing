#!/usr/bin/bash

#gcc -I. -o netconf netconf.c -lev -lassh -lcbor -lcjson
gcc -I. -o netconf netconf.c -lev -lcbor -lcjson -lassh -g
