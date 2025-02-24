#!/usr/bin/bash

#gcc -I. -o netconf netconf.c -lev -lassh -lcbor -lcjson
./generate_netconf_gperf.pl
gperf nx_parse.gperf > nx_parse.h
#gcc -I. -c -fPIC nx_parse.c -o nx_parse.o
#gcc -shared -o libnx_parse.so nx_parse.o
#gcc -I. -o netconf netconf.c -lev -lcbor -lcjson -lassh -lexpat -L -lnx_parse -g
gcc -I. -o netconf netconf.c -lev -lcbor -lcjson -lassh -lexpat -g
