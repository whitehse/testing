#!/bin/bash

dumpcap -i 1 -s0 -b filesize:16384 -b files:1024 -w ring.cap
