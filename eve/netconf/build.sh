#!/usr/bin/bash

./generate_netconf_gperf.pl

gperf --enum -t nx_parse.gperf > nx_parse.c
gcc -I. -c -fPIC nx_parse.c -o nx_parse.o
gcc -shared -o libnx_parse.so nx_parse.o
ar rcs libnx_parse.a nx_parse.o

gperf --enum -t nx_xmlns_parse.gperf > nx_xmlns_parse.c
gcc -I. -c -fPIC nx_xmlns_parse.c -o nx_xmlns_parse.o
gcc -shared -o libnx_xmlns_parse.so nx_xmlns_parse.o
ar rcs libnx_xmlns_parse.a nx_xmlns_parse.o

gcc -I. -o netconf netconf.c -lev -lcbor -lcjson -lassh -lexpat -L. -l:libnx_parse.a -l:libnx_xmlns_parse.a -g
