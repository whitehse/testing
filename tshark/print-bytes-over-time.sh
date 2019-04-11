#!/bin/bash

tshark -qz io,stat,10,ip,icmp,udp,tcp
