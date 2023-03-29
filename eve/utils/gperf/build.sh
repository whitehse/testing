#!/usr/bin/bash

#gperf -t test.gperf > test.h
#gcc -I. -o test test.c

#gperf -s 10 -t calix.gperf > calix.h
gperf -t calix.gperf > calix.h
gcc -I. -o calix calix.c

