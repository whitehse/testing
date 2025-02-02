#!/usr/bin/bash

gcc -I. -o netconf netconf.c -lev -lassh -lcbor -lcjson
