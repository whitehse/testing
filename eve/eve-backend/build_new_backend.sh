#!/usr/bin/bash

gcc -I. -o ssh_aggregator ssh_aggregator.c -lev -lassh -lcbor -lcjson
