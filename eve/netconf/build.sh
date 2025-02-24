#!/usr/bin/bash

#gcc -I. -o netconf netconf.c -lev -lassh -lcbor -lcjson
./generate_netconf_gperf.pl
gperf -s 10 -t nx_parse.gperf > nx_parse.c
#gperf --enum -t nx_parse.gperf > nx_parse.c
gcc -I. -c -fPIC nx_parse.c -o nx_parse.o
gcc -shared -o libnx_parse.so nx_parse.o
ar rcs libnx_parse.a nx_parse.o
gcc -I. -o netconf netconf.c -lev -lcbor -lcjson -lassh -lexpat -L. -l:libnx_parse.a -g
#gcc -I. -o netconf netconf.c -lev -lcbor -lcjson -lassh -lexpat -g
