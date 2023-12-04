#!/usr/bin/bash

##gperf -t test.gperf > test.h
##gcc -I. -o test test.c

#gperf -s 10 -t calix.gperf > calix.h
##gperf -t calix.gperf > calix.h
#gperf -t calix_perceived_severity.gperf > calix_perceived_severity.c
##gcc -I. -o calix calix_perceived_severity.c calix.c
#gcc -I. -c calix_perceived_severity.c
#ar rcs eve_hash_functions.a calix_perceived_severity.o
#gcc -I. -o calix calix.c eve_hash_functions.a

#gperf -s 10 -t calix.gperf > calix.h
gperf -s 10 -t calix_axos.gperf > calix_axos.c
#gperf -t calix.gperf > calix.h
gperf -t calix_perceived_severity.gperf > calix_perceived_severity.c
gcc -I. -o calix calix_axos.c calix_perceived_severity.c calix.c
